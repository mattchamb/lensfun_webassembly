/*
    Private constructors and destructors
    Copyright (C) 2007 by Andrew Zabolotny
*/

#include "config.h"
#include "lensfun.h"
#include "lensfunprv.h"
#include <limits.h>
#include <stdlib.h>
#include "../../include/regex/regex.h"
#include <string.h>
#include <locale.h>
#include <math.h>
#include "windows/mathconstants.h"
#include <algorithm>

typedef unsigned char guchar;

static struct
{
    const char *regex;
    guchar matchidx [3];
    bool compiled;
    regex_t rex;
} lens_name_regex [] =
{
    {
        // [min focal]-[max focal]mm f/[min aperture]-[max aperture]
        "([[:space:]]+|^)([0-9]+[0-9.]*)(-[0-9]+[0-9.]*)?(mm)?[[:space:]]+(f/|f|1/|1:)?([0-9.]+)(-[0-9.]+)?",
        { 2, 3, 6 },
        false
    },
    {
        // 1:[min aperture]-[max aperture] [min focal]-[max focal]mm
        "[[:space:]]+1:([0-9.]+)(-[0-9.]+)?[[:space:]]+([0-9.]+)(-[0-9.]+)?(mm)?",
        { 3, 4, 1 },
        false
    },
    {
        // [min aperture]-[max aperture]/[min focal]-[max focal]
        "([0-9.]+)(-[0-9.]+)?[[:space:]]*/[[:space:]]*([0-9.]+)(-[0-9.]+)?",
        { 3, 4, 1 },
        false
    },
};

static struct
{
    bool compiled;
    regex_t rex;
} extender_magnification_regex =
{
    false
};

static float _lf_parse_float (const char *model, const regmatch_t &match)
{
    char tmp [100];
    const char *src = model + match.rm_so;
    int len = match.rm_eo - match.rm_so;

    // Skip '-' since it's not a minus sign but rather the separator
    if (*src == '-')
        src++, len--;
    strncpy (tmp, src, len);
    tmp [len] = 0;

    return atof (tmp);
}

static bool _lf_parse_lens_name (const char *model,
                                 float &minf, float &maxf,
                                 float &mina)
{
    if (!model)
        return false;

    for (size_t i = 0; i < ARRAY_LEN (lens_name_regex); i++)
    {
        if (!lens_name_regex [i].compiled)
        {
            regcomp (&lens_name_regex [i].rex, lens_name_regex [i].regex,
                     REG_EXTENDED | REG_ICASE);
            lens_name_regex [i].compiled = true;
        }

        regmatch_t matches [10];
        if (regexec (&lens_name_regex [i].rex, model, 10, matches, 0))
            continue;

        guchar *matchidx = lens_name_regex [i].matchidx;
        if (matches [matchidx [0]].rm_so != -1)
            minf = _lf_parse_float (model, matches [matchidx [0]]);
        if (matches [matchidx [1]].rm_so != -1)
            maxf = _lf_parse_float (model, matches [matchidx [1]]);
        if (matches [matchidx [2]].rm_so != -1)
            mina = _lf_parse_float (model, matches [matchidx [2]]);
        return true;
    }

    return false;
}

static void _lf_free_lens_regex ()
{
    for (size_t i = 0; i < ARRAY_LEN (lens_name_regex); i++)
        if (lens_name_regex [i].compiled)
        {
            regfree (&lens_name_regex [i].rex);
            lens_name_regex [i].compiled = false;
        }
}

//------------------------------------------------------------------------//

static int _lf_lens_regex_refs = 0;

lfLens::lfLens ()
{
    // Defaults for attributes are "unknown" (mostly 0).  Otherwise, ad hoc
    // lfLens instances used for searches could not be matched against database
    // lenses easily.  If you need defaults for database tags, set them when
    // reading the database.
    memset (this, 0, sizeof (*this));
    Type = LF_UNKNOWN;
    _lf_lens_regex_refs++;
}

lfLens::~lfLens ()
{
    lf_free (Maker);
    lf_free (Model);
    _lf_list_free ((void **)Mounts);
    _lf_list_free ((void **)CalibDistortion);
    _lf_list_free ((void **)CalibTCA);
    _lf_list_free ((void **)CalibVignetting);
    _lf_list_free ((void **)CalibCrop);
    _lf_list_free ((void **)CalibFov);
    if (!--_lf_lens_regex_refs)
        _lf_free_lens_regex ();
}

lfLens::lfLens (const lfLens &other)
{
    Maker = lf_mlstr_dup (other.Maker);
    Model = lf_mlstr_dup (other.Model);
    MinFocal = other.MinFocal;
    MaxFocal = other.MaxFocal;
    MinAperture = other.MinAperture;
    MaxAperture = other.MaxAperture;

    Mounts = NULL;
    if (other.Mounts)
        for (int i = 0; other.Mounts [i]; i++)
            AddMount (other.Mounts [i]);

    CenterX = other.CenterX;
    CenterY = other.CenterY;
    CropFactor = other.CropFactor;
    AspectRatio = other.AspectRatio;
    Type = other.Type;

    CalibDistortion = NULL;
    if (other.CalibDistortion)
        for (int i = 0; other.CalibDistortion [i]; i++)
            AddCalibDistortion (other.CalibDistortion [i]);
    CalibTCA = NULL;
    if (other.CalibTCA)
        for (int i = 0; other.CalibTCA [i]; i++)
            AddCalibTCA (other.CalibTCA [i]);
    CalibVignetting = NULL;
    if (other.CalibVignetting)
        for (int i = 0; other.CalibVignetting [i]; i++)
            AddCalibVignetting (other.CalibVignetting [i]);
    CalibCrop = NULL;
    if (other.CalibCrop)
        for (int i = 0; other.CalibCrop [i]; i++)
            AddCalibCrop (other.CalibCrop [i]);
    CalibFov = NULL;
    if (other.CalibFov)
        for (int i = 0; other.CalibFov [i]; i++)
            AddCalibFov (other.CalibFov [i]);
}

lfLens &lfLens::operator = (const lfLens &other)
{
    lf_free (Maker);
    Maker = lf_mlstr_dup (other.Maker);
    lf_free (Model);
    Model = lf_mlstr_dup (other.Model);
    MinFocal = other.MinFocal;
    MaxFocal = other.MaxFocal;
    MinAperture = other.MinAperture;
    MaxAperture = other.MaxAperture;

    lf_free (Mounts); Mounts = NULL;
    if (other.Mounts)
        for (int i = 0; other.Mounts [i]; i++)
            AddMount (other.Mounts [i]);

    CenterX = other.CenterX;
    CenterY = other.CenterY;
    CropFactor = other.CropFactor;
    AspectRatio = other.AspectRatio;
    Type = other.Type;

    lf_free (CalibDistortion); CalibDistortion = NULL;
    if (other.CalibDistortion)
        for (int i = 0; other.CalibDistortion [i]; i++)
            AddCalibDistortion (other.CalibDistortion [i]);
    lf_free (CalibTCA); CalibTCA = NULL;
    if (other.CalibTCA)
        for (int i = 0; other.CalibTCA [i]; i++)
            AddCalibTCA (other.CalibTCA [i]);
    lf_free (CalibVignetting); CalibVignetting = NULL;
    if (other.CalibVignetting)
        for (int i = 0; other.CalibVignetting [i]; i++)
            AddCalibVignetting (other.CalibVignetting [i]);
    lf_free (CalibCrop); CalibCrop = NULL;
    if (other.CalibCrop)
        for (int i = 0; other.CalibCrop [i]; i++)
            AddCalibCrop (other.CalibCrop [i]);
    lf_free (CalibFov); CalibFov = NULL;
    if (other.CalibFov)
        for (int i = 0; other.CalibFov [i]; i++)
            AddCalibFov (other.CalibFov [i]);

    return *this;
}

void lfLens::SetMaker (const char *val, const char *lang)
{
    Maker = lf_mlstr_add (Maker, lang, val);
}

void lfLens::SetModel (const char *val, const char *lang)
{
    Model = lf_mlstr_add (Model, lang, val);
}

void lfLens::AddMount (const char *val)
{
    if (val)
        _lf_addstr (&Mounts, val);
}

void lfLens::GuessParameters ()
{
    if (!extender_magnification_regex.compiled)
    {
        regcomp (&extender_magnification_regex.rex, "[0-9](\\.[0.9]+)?x",
                 REG_EXTENDED | REG_ICASE);
        extender_magnification_regex.compiled = true;
    }

    float minf = float (INT_MAX), maxf = float (INT_MIN);
    float mina = float (INT_MAX), maxa = float (INT_MIN);

    char *old_numeric = setlocale (LC_NUMERIC, NULL);
    old_numeric = strdup (old_numeric);
    setlocale (LC_NUMERIC, "C");

    if (Model && (!MinAperture || !MinFocal) &&
        !strstr (Model, "adapter") &&
        !strstr (Model, "reducer") &&
        !strstr (Model, "booster") &&
        !strstr (Model, "extender") &&
        !strstr (Model, "converter") &&
        regexec (&extender_magnification_regex.rex, Model, 0, NULL, 0))
        _lf_parse_lens_name (Model, minf, maxf, mina);

    if (!MinAperture || !MinFocal)
    {
        // Try to find out the range of focal lengths using calibration data
        if (CalibDistortion)
            for (int i = 0; CalibDistortion [i]; i++)
            {
                float f = CalibDistortion [i]->Focal;
                if (f < minf)
                    minf = f;
                if (f > maxf)
                    maxf = f;
            }
        if (CalibTCA)
            for (int i = 0; CalibTCA [i]; i++)
            {
                float f = CalibTCA [i]->Focal;
                if (f < minf)
                    minf = f;
                if (f > maxf)
                    maxf = f;
            }
        if (CalibVignetting)
            for (int i = 0; CalibVignetting [i]; i++)
            {
                float f = CalibVignetting [i]->Focal;
                float a = CalibVignetting [i]->Aperture;
                if (f < minf)
                    minf = f;
                if (f > maxf)
                    maxf = f;
                if (a < mina)
                    mina = a;
                if (a > maxa)
                    maxa = a;
            }
        if (CalibCrop)
            for (int i=0; CalibCrop [i]; i++)
            {
                float f = CalibCrop [i]->Focal;
                if (f < minf)
                    minf = f;
                if (f > maxf)
                    maxf = f;
            }
        if (CalibFov)
            for (int i=0; CalibFov [i]; i++)
            {
                float f = CalibFov [i]->Focal;
                if (f < minf)
                    minf = f;
                if (f > maxf)
                    maxf = f;
            }

    }

    if (minf != INT_MAX && !MinFocal)
        MinFocal = minf;
    if (maxf != INT_MIN && !MaxFocal)
        MaxFocal = maxf;
    if (mina != INT_MAX && !MinAperture)
        MinAperture = mina;
    if (maxa != INT_MIN && !MaxAperture)
        MaxAperture = maxa;

    if (!MaxFocal)
        MaxFocal = MinFocal;

    setlocale (LC_NUMERIC, old_numeric);
    free (old_numeric);
}

bool lfLens::Check ()
{
    GuessParameters ();

    if (!Model || !Mounts || CropFactor <= 0 ||
        MinFocal > MaxFocal || (MaxAperture && MinAperture > MaxAperture) ||
        AspectRatio < 1)
        return false;

    return true;
}

const char *lfLens::GetDistortionModelDesc (
    lfDistortionModel model, const char **details, const lfParameter ***params)
{
    static const lfParameter *param_none [] = { NULL };

    static const lfParameter param_poly3_k1 = { "k1", -0.2F, 0.2F, 0.0F };
    static const lfParameter *param_poly3 [] = { &param_poly3_k1, NULL };

    static const lfParameter param_poly5_k2 = { "k2", -0.2F, 0.2F, 0.0F };
    static const lfParameter *param_poly5 [] = { &param_poly3_k1, &param_poly5_k2, NULL };

    static const lfParameter param_ptlens_a = { "a", -0.5F, 0.5F, 0.0F };
    static const lfParameter param_ptlens_b = { "b", -1.0F, 1.0F, 0.0F };
    static const lfParameter param_ptlens_c = { "c", -1.0F, 1.0F, 0.0F };
    static const lfParameter *param_ptlens [] = {
        &param_ptlens_a, &param_ptlens_b, &param_ptlens_c, NULL };

    static const lfParameter param_acm_k3 = { "k3", -1.0F, 1.0F, 0.0F };
    static const lfParameter param_acm_k4 = { "k4", -1.0F, 1.0F, 0.0F };
    static const lfParameter param_acm_k5 = { "k5", -1.0F, 1.0F, 0.0F };
    static const lfParameter *param_acm [] = {
        &param_poly3_k1, &param_poly5_k2, &param_acm_k3,
        &param_acm_k4, &param_acm_k5, NULL };

    switch (model)
    {
        case LF_DIST_MODEL_NONE:
            if (details)
                *details = "No distortion model";
            if (params)
                *params = param_none;
            return "None";

        case LF_DIST_MODEL_POLY3:
            if (details)
                *details = "Rd = Ru * (1 - k1 + k1 * Ru^2)\n"
                    "Ref: http://www.imatest.com/docs/distortion.html";
            if (params)
                *params = param_poly3;
            return "3rd order polynomial";

        case LF_DIST_MODEL_POLY5:
            if (details)
                *details = "Rd = Ru * (1 + k1 * Ru^2 + k2 * Ru^4)\n"
                    "Ref: http://www.imatest.com/docs/distortion.html";
            if (params)
                *params = param_poly5;
            return "5th order polynomial";

        case LF_DIST_MODEL_PTLENS:
            if (details)
                *details = "Rd = Ru * (a * Ru^3 + b * Ru^2 + c * Ru + 1 - (a + b + c))\n"
                    "Ref: http://wiki.panotools.org/Lens_correction_model";
            if (params)
                *params = param_ptlens;
            return "PanoTools lens model";

        case LF_DIST_MODEL_ACM:
            if (details)
                *details = "x_d = x_u (1 + k_1 r^2 + k_2 r^4 + k_3 r^6) + 2x(k_4y + k_5x) + k_5 r^2\n"
                    "y_d = y_u (1 + k_1 r^2 + k_2 r^4 + k_3 r^6) + 2y(k_4y + k_5x) + k_4 r^2\n"
                    "Coordinates are in units of focal length.\n"
                    "Ref: http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf";
            if (params)
                *params = param_acm;
            return "Adobe camera model";

        default:
            // keep gcc 4.4 happy
            break;
    }

    if (details)
        *details = NULL;
    if (params)
        *params = NULL;
    return NULL;
}

const char *lfLens::GetTCAModelDesc (
    lfTCAModel model, const char **details, const lfParameter ***params)
{
    static const lfParameter *param_none [] = { NULL };

    static const lfParameter param_linear_kr = { "kr", 0.99F, 1.01F, 1.0F };
    static const lfParameter param_linear_kb = { "kb", 0.99F, 1.01F, 1.0F };
    static const lfParameter *param_linear [] =
    { &param_linear_kr, &param_linear_kb, NULL };

    static const lfParameter param_poly3_br = { "br", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_poly3_cr = { "cr", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_poly3_vr = { "vr",  0.99F, 1.01F, 1.0F };
    static const lfParameter param_poly3_bb = { "bb", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_poly3_cb = { "cb", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_poly3_vb = { "vb",  0.99F, 1.01F, 1.0F };
    static const lfParameter *param_poly3 [] =
    {
        &param_poly3_vr, &param_poly3_vb,
        &param_poly3_cr, &param_poly3_cb,
        &param_poly3_br, &param_poly3_bb,
        NULL
    };

    static const lfParameter param_acm_alpha0 = { "alpha0", 0.99F, 1.01F, 1.0F };
    static const lfParameter param_acm_beta0 = { "beta0", 0.99F, 1.01F, 1.0F };
    static const lfParameter param_acm_alpha1 = { "alpha1", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_beta1 = { "beta1", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_alpha2 = { "alpha2", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_beta2 = { "beta2", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_alpha3 = { "alpha3", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_beta3 = { "beta3", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_alpha4 = { "alpha4", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_beta4 = { "beta4", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_alpha5 = { "alpha5", -0.01F, 0.01F, 0.0F };
    static const lfParameter param_acm_beta5 = { "beta5", -0.01F, 0.01F, 0.0F };
    static const lfParameter *param_acm [] =
    {
        &param_acm_alpha0, &param_acm_beta0,
        &param_acm_alpha1, &param_acm_beta1,
        &param_acm_alpha2, &param_acm_beta2,
        &param_acm_alpha3, &param_acm_beta3,
        &param_acm_alpha4, &param_acm_beta4,
        &param_acm_alpha5, &param_acm_beta5,
        NULL
    };

    switch (model)
    {
        case LF_TCA_MODEL_NONE:
            if (details)
                *details = "No transversal chromatic aberration model";
            if (params)
                *params = param_none;
            return "None";

        case LF_TCA_MODEL_LINEAR:
            if (details)
                *details = "Cd = Cs * k\n"
                    "Ref: http://cipa.icomos.org/fileadmin/papers/Torino2005/403.pdf";
            if (params)
                *params = param_linear;
            return "Linear";

        case LF_TCA_MODEL_POLY3:
            if (details)
                *details = "Cd = Cs^3 * b + Cs^2 * c + Cs * v\n"
                    "Ref: http://wiki.panotools.org/Tca_correct";
            if (params)
                *params = param_poly3;
            return "3rd order polynomial";

        case LF_TCA_MODEL_ACM:
            if (details)
                *details = "x_{d,R} = α_0 ((1 + α_1 r_{u,R}^2 + α_2 r_{u,R}^4 + α_3 r_{u,R}^6) x_{u,R} +\n"
                           "          2(α_4 y_{u,R} + α_5 x_{u,R}) x_{u,R} + α_5 r_{u,R}^2)\n"
                           "y_{d,R} = α_0 ((1 + α_1 r_{u,R}^2 + α_2 r_{u,R}^4 + α_3 r_{u,R}^6) y_{u,R} +\n"
                           "          2(α_4 y_{u,R} + α_5 x_{u,R}) y_{u,R} + α_4 r_{u,R}^2)\n"
                           "x_{d,B} = β_0 ((1 + β_1 r_{u,B}^2 + β_2 r_{u,B}^4 + β_3 r_{u,B}^6) x_{u,B} +\n"
                           "          2(β_4 y_{u,B} + β_5 x_{u,B}) x_{u,B} + β_5 r_{u,B}^2)\n"
                           "y_{d,B} = β_0 ((1 + β_1 r_{u,B}^2 + β_2 r_{u,B}^4 + β_3 r_{u,B}^6) y_{u,B} +\n"
                           "          2(β_4 y_{u,B} + β_5 x_{u,B}) y_{u,B} + β_4 r_{u,B}^2)\n"
                    "Ref: http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf";
            if (params)
                *params = param_acm;
            return "Adobe camera model";

        default:
            // keep gcc 4.4 happy
            break;
    }

    if (details)
        *details = NULL;
    if (params)
        *params = NULL;
    return NULL;
}

const char *lfLens::GetVignettingModelDesc (
    lfVignettingModel model, const char **details, const lfParameter ***params)
{
    static const lfParameter *param_none [] = { NULL };

    static const lfParameter param_pa_k1 = { "k1", -3.0, 1.0, 0.0 };
    static const lfParameter param_pa_k2 = { "k2", -5.0, 10.0, 0.0 };
    static const lfParameter param_pa_k3 = { "k3", -5.0, 10.0, 0.0 };
    static const lfParameter *param_pa [] =
    { &param_pa_k1, &param_pa_k2, &param_pa_k3, NULL };

    static const lfParameter param_acm_alpha1 = { "alpha1", -1.0, 1.0, 0.0 };
    static const lfParameter param_acm_alpha2 = { "alpha2", -5.0, 10.0, 0.0 };
    static const lfParameter param_acm_alpha3 = { "alpha3", -5.0, 10.0, 0.0 };
    static const lfParameter *param_acm [] =
    { &param_acm_alpha1, &param_acm_alpha2, &param_acm_alpha3, NULL };

    switch (model)
    {
        case LF_VIGNETTING_MODEL_NONE:
            if (details)
                *details = "No vignetting model";
            if (params)
                *params = param_none;
            return "None";

        case LF_VIGNETTING_MODEL_PA:
            if (details)
                *details = "Pablo D'Angelo vignetting model\n"
                    "(which is a more general variant of the cos^4 law):\n"
                    "Cd = Cs * (1 + k1 * R^2 + k2 * R^4 + k3 * R^6)\n"
                    "Ref: http://hugin.sourceforge.net/tech/";
            if (params)
                *params = param_pa;
            return "6th order polynomial (Pablo D'Angelo)";

        case LF_VIGNETTING_MODEL_ACM:
            if (details)
                *details = "Adobe's vignetting model\n"
                    "(which differs from D'Angelo's only in the coordinate system):\n"
                    "Cd = Cs * (1 + k1 * R^2 + k2 * R^4 + k3 * R^6)\n"
                    "Ref: http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf";
            if (params)
                *params = param_acm;
            return "6th order polynomial (Adobe)";

        default:
            // keep gcc 4.4 happy
            break;
    }

    if (details)
        *details = "";
    if (params)
        *params = NULL;
    return NULL;
}

const char *lfLens::GetCropDesc (
    lfCropMode mode , const char **details, const lfParameter ***params)
{
    static const lfParameter *param_none [] = { NULL };

    static const lfParameter param_crop_left = { "left", -1.0F, 1.0F, 0.0F };
    static const lfParameter param_crop_right = { "right", 0.0F, 2.0F, 0.0F };
    static const lfParameter param_crop_top = { "top", -1.0F, 1.0F, 0.0F };
    static const lfParameter param_crop_bottom = { "bottom", 0.0F, 2.0F, 0.0F };
    static const lfParameter *param_crop [] = { &param_crop_left, &param_crop_right, &param_crop_top, &param_crop_bottom, NULL };

    switch (mode)
    {
        case LF_NO_CROP:
            if (details)
                *details = "No crop";
            if (params)
                *params = param_none;
            return "No crop";

        case LF_CROP_RECTANGLE:
            if (details)
                *details = "Rectangular crop area";
            if (params)
                *params = param_crop;
            return "rectangular crop";

        case LF_CROP_CIRCLE:
            if (details)
                *details = "Circular crop area";
            if (params)
                *params = param_crop;
            return "circular crop";

        default:
            // keep gcc 4.4 happy
            break;
    }

    if (details)
        *details = NULL;
    if (params)
        *params = NULL;
    return NULL;
}

const char *lfLens::GetLensTypeDesc (lfLensType type, const char **details)
{
    switch (type)
    {
        case LF_UNKNOWN:
            if (details)
                *details = "";
            return "Unknown";

        case LF_RECTILINEAR:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Rectilinear_Projection";
            return "Rectilinear";

        case LF_FISHEYE:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Fisheye_Projection";
            return "Fish-Eye";

        case LF_PANORAMIC:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Cylindrical_Projection";
            return "Panoramic";

        case LF_EQUIRECTANGULAR:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Equirectangular_Projection";
            return "Equirectangular";

        case LF_FISHEYE_ORTHOGRAPHIC:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Fisheye_Projection";
            return "Fisheye, orthographic";

        case LF_FISHEYE_STEREOGRAPHIC:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Stereographic_Projection";
            return "Fisheye, stereographic";

        case LF_FISHEYE_EQUISOLID:
            if (details)
                *details = "Ref: http://wiki.panotools.org/Fisheye_Projection";
            return "Fisheye, equisolid";

        case LF_FISHEYE_THOBY:
            if (details)
                *details = "Ref: http://groups.google.com/group/hugin-ptx/browse_thread/thread/bd822d178e3e239d";
            return "Thoby-Fisheye";

        default:
            // keep gcc 4.4 happy
            break;
    }

    if (details)
        *details = "";
    return NULL;
}

static bool cmp_distortion (const void *x1, const void *x2)
{
    const lfLensCalibDistortion *d1 = static_cast<const lfLensCalibDistortion *> (x1);
    const lfLensCalibDistortion *d2 = static_cast<const lfLensCalibDistortion *> (x2);
    return (d1->Focal == d2->Focal);
}

void lfLens::AddCalibDistortion (const lfLensCalibDistortion *dc)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibDistortion ***cd;
        void ***arr;
    } x = { &CalibDistortion };
    _lf_addobj (x.arr, dc, sizeof (*dc), cmp_distortion);
}

bool lfLens::RemoveCalibDistortion (int idx)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibDistortion ***cd;
        void ***arr;
    } x = { &CalibDistortion };
    return _lf_delobj (x.arr, idx);
}

static bool cmp_tca (const void *x1, const void *x2)
{
    const lfLensCalibTCA *t1 = static_cast<const lfLensCalibTCA *> (x1);
    const lfLensCalibTCA *t2 = static_cast<const lfLensCalibTCA *> (x2);
    return (t1->Focal == t2->Focal);
}

void lfLens::AddCalibTCA (const lfLensCalibTCA *tcac)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibTCA ***ctca;
        void ***arr;
    } x = { &CalibTCA };
    _lf_addobj (x.arr, tcac, sizeof (*tcac), cmp_tca);
}

bool lfLens::RemoveCalibTCA (int idx)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibTCA ***ctca;
        void ***arr;
    } x = { &CalibTCA };
    return _lf_delobj (x.arr, idx);
}

static bool cmp_vignetting (const void *x1, const void *x2)
{
    const lfLensCalibVignetting *v1 = static_cast<const lfLensCalibVignetting *> (x1);
    const lfLensCalibVignetting *v2 = static_cast<const lfLensCalibVignetting *> (x2);
    return (v1->Focal == v2->Focal) &&
           (v1->Distance == v2->Distance) &&
           (v1->Aperture == v2->Aperture);
}

void lfLens::AddCalibVignetting (const lfLensCalibVignetting *vc)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibVignetting ***cv;
        void ***arr;
    } x = { &CalibVignetting };
    _lf_addobj (x.arr, vc, sizeof (*vc), cmp_vignetting);
}

bool lfLens::RemoveCalibVignetting (int idx)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibVignetting ***cv;
        void ***arr;
    } x = { &CalibVignetting };
    return _lf_delobj (x.arr, idx);
}

static bool cmp_lenscrop (const void *x1, const void *x2)
{
    const lfLensCalibCrop *d1 = static_cast<const lfLensCalibCrop *> (x1);
    const lfLensCalibCrop *d2 = static_cast<const lfLensCalibCrop *> (x2);
    return (d1->Focal == d2->Focal);
}

void lfLens::AddCalibCrop (const lfLensCalibCrop *lcc)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibCrop ***cd;
        void ***arr;
    } x = { &CalibCrop };
    _lf_addobj (x.arr, lcc, sizeof (*lcc), cmp_lenscrop);
}

bool lfLens::RemoveCalibCrop (int idx)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibCrop ***cd;
        void ***arr;
    } x = { &CalibCrop };
    return _lf_delobj (x.arr, idx);
}

static bool cmp_lensfov (const void *x1, const void *x2)
{
    const lfLensCalibFov *d1 = static_cast<const lfLensCalibFov *> (x1);
    const lfLensCalibFov *d2 = static_cast<const lfLensCalibFov *> (x2);
    return (d1->Focal == d2->Focal);
}

void lfLens::AddCalibFov (const lfLensCalibFov *lcf)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibFov ***cd;
        void ***arr;
    } x = { &CalibFov };
    _lf_addobj (x.arr, lcf, sizeof (*lcf), cmp_lensfov);
}

bool lfLens::RemoveCalibFov (int idx)
{
    // Avoid "dereferencing type-punned pointer will break strict-aliasing rules" warning
    union
    {
        lfLensCalibFov ***cd;
        void ***arr;
    } x = { &CalibFov };
    return _lf_delobj (x.arr, idx);
}

static int __insert_spline (void **spline, float *spline_dist, float dist, void *val)
{
    if (dist < 0)
    {
        if (dist > spline_dist [1])
        {
            spline_dist [0] = spline_dist [1];
            spline_dist [1] = dist;
            spline [0] = spline [1];
            spline [1] = val;
            return 1;
        }
        else if (dist > spline_dist [0])
        {
            spline_dist [0] = dist;
            spline [0] = val;
            return 0;
        }
    }
    else
    {
        if (dist < spline_dist [2])
        {
            spline_dist [3] = spline_dist [2];
            spline_dist [2] = dist;
            spline [3] = spline [2];
            spline [2] = val;
            return 2;
        }
        else if (dist < spline_dist [3])
        {
            spline_dist [3] = dist;
            spline [3] = val;
            return 3;
        }
    }
    return -1;
}

/* Coefficient interpolation

   The interpolation of model coefficients bases on spline interpolation
   (distortion/TCA) and the IDW algorithm (vignetting).  Both methods work best
   if the input data points (i.e. the values in the XML file) are
   preconditioned so that they follow more or less a linear slope.  For this
   preconditioning, we transform the axes.

   First, let's have a look at the untransformed slopes.  For distortion, the
   parameters decrease with increasing focal length, following a 1/f law.  The
   same is true for all TCA parameters besides the zeroth one (i.e. the term
   close to 1), which remains constant.  In contrast, all three vignetting
   parameters remain constant for different focal lengths, however, they do
   decrease according to 1/aperture and 1/distance.  All of this is only roughly
   true of course, however, I could really reproduce it by mass-analysing the
   existing data.

   Thus, in order to make the slopes linear, I transform the aperture and
   distance axes to reciprocal axes in __vignetting_dist ().  I keep the focal
   length axis linear, though, for all three kinds of correction.  Instead, I
   transform the parameter axis by multiplying it by the focal length of the
   respective data point for those parameters that exhibit 1/f behaviour.  Of
   course, this parameter scaling must be undone after the interpolation by
   dividing by the destination focal length.

   The ACM models are a special case because they use a coordinate system that
   scales with the focal length.  Therefore, their parameters tend to increase
   (partly heavily) by the focal length.  I undo this by dividing the
   parameters by the focal length in the same power that the respective r
   coordinate has.  For example, I divide k₁ of the distortion model by f²,
   because it says “k₁r²” in the formula.  Note that this is done *on top* of
   the multiplying by f, so that effectively, k₁ is divided by f.  Also note
   that the factor x in the Adobe formula does not mean that we have r³,
   because the left-hand side of the formula scales by f, too, and anhilates
   this factor.

   All of this only works well with polynomial-based models.  I simply don't
   know whether or how it applies to completely different types of models.

   Does it really matter?  Well, my own research revealed that only for
   vignetting, it is absolutely necessary.  Especially the parameter scaling
   for the ACM model prevents the slightly fragile IDW algorithm from failing
   badly in many cases.  The reciprocal axis for aperture also helps a lot.  I
   think that the parameter scaling for distortion and TCA for the ACM models
   is also advantageous since especially the k₃ parameter tends to explode by
   the focal length, and the splines will have a hard time to follow such
   slopes.  Well, and if you implement the facilities for axis transformation
   anyway, you can cover all other cases too, I think.


   The function __parameter_scales () below implements the parameter axis
   scaling.  If returns factors by which the parameters are multiplied, and
   divided by after the interpolation.  It takes the "type" of the correction
   (distortion/TCA/vignetting), the model, and the parameter index in this
   model.  All of these parameters should be easy to understand.

   The "values" parameter is slightly more convoluted.  It takes the focal
   lengths for which scale factors should be returned.  "number_of_values"
   contains the number of values in the "values" array.  At the same time,
   "values" contains the scale factors themselves, thus, __parameter_scales ()
   changes the "values" array in place.  This makes sense because the caller
   doesn't need the focal lengths anymore, and because in most cases, the scale
   factor is the focal length itself, so nothing needs to be changed.
 */
static void __parameter_scales (float values [], int number_of_values,
                                int type, int model, int index)
{
    switch (type)
    {
    case LF_MODIFY_DISTORTION:
        switch (model)
        {
        case LF_DIST_MODEL_POLY3:
        case LF_DIST_MODEL_POLY5:
        case LF_DIST_MODEL_PTLENS:
            break;

        case LF_DIST_MODEL_ACM:
            const float exponent = index < 3 ? (float)(2 * (index + 1)) : 1.0;
            for (int i=0; i < number_of_values; i++)
                values [i] /= pow (values [i], exponent);
            break;
        }
        break;

    case LF_MODIFY_TCA:
        switch (model)
        {
        case LF_TCA_MODEL_LINEAR:
        case LF_TCA_MODEL_POLY3:
            if (index < 2)
                for (int i=0; i < number_of_values; i++)
                    values [i] = 1.0;
            break;

        case LF_TCA_MODEL_ACM:
            const float exponent = index > 1 && index < 8 ? (float)(index / 2 * 2) : 1.0;
            for (int i=0; i < number_of_values; i++)
                values [i] /= pow (values [i], exponent);
            break;
        }
        break;

    case LF_MODIFY_VIGNETTING:
        switch (model)
        {
        case LF_VIGNETTING_MODEL_PA:
            for (int i=0; i < number_of_values; i++)
                values [i] = 1.0;
            break;

        case LF_VIGNETTING_MODEL_ACM:
            const float exponent = (float)(2 * (index + 1));
            for (int i=0; i < number_of_values; i++)
                values [i] = 1.0 / pow (values [i], exponent);
            break;
        }
    }
}

bool lfLens::InterpolateDistortion (float focal, lfLensCalibDistortion &res) const
{
    if (!CalibDistortion)
        return false;

    union
    {
        lfLensCalibDistortion *spline [4];
        void *spline_ptr [4];
    };
    float spline_dist [4] = { -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };
    lfDistortionModel dm = LF_DIST_MODEL_NONE;

    memset (spline, 0, sizeof (spline));
    for (int i = 0; CalibDistortion [i]; i++)
    {
        lfLensCalibDistortion *c = CalibDistortion [i];
        if (c->Model == LF_DIST_MODEL_NONE)
            continue;

        // Take into account just the first encountered lens model
        if (dm == LF_DIST_MODEL_NONE)
            dm = c->Model;
        else if (dm != c->Model)
        {
            continue;
        }

        float df = focal - c->Focal;
        if (df == 0.0)
        {
            // Exact match found, don't care to interpolate
            res = *c;
            return true;
        }

        __insert_spline (spline_ptr, spline_dist, df, c);
    }

    if (!spline [1] || !spline [2])
    {
        if (spline [1])
            res = *spline [1];
        else if (spline [2])
            res = *spline [2];
        else
            return false;

        return true;
    }

    // No exact match found, interpolate the model parameters
    res.Model = dm;
    res.Focal = focal;

    float t = (focal - spline [1]->Focal) / (spline [2]->Focal - spline [1]->Focal);

    res.RealFocal = _lf_interpolate (
        spline [0] ? spline [0]->RealFocal : FLT_MAX,
        spline [1]->RealFocal, spline [2]->RealFocal,
        spline [3] ? spline [3]->RealFocal : FLT_MAX, t);
    for (size_t i = 0; i < ARRAY_LEN (res.Terms); i++)
    {
        float values [5] = {spline [0] ? spline [0]->Focal : NAN, spline [1]->Focal,
                            spline [2]->Focal, spline [3] ? spline [3]->Focal : NAN,
                            focal};
        __parameter_scales (values, 5, LF_MODIFY_DISTORTION, dm, i);
        res.Terms [i] = _lf_interpolate (
            spline [0] ? spline [0]->Terms [i] * values [0] : FLT_MAX,
            spline [1]->Terms [i] * values [1], spline [2]->Terms [i] * values [2],
            spline [3] ? spline [3]->Terms [i] * values [3] : FLT_MAX,
            t) / values [4];
    }

    return true;
}

bool lfLens::InterpolateTCA (float focal, lfLensCalibTCA &res) const
{
    if (!CalibTCA)
        return false;

    union
    {
        lfLensCalibTCA *spline [4];
        void *spline_ptr [4];
    };
    float spline_dist [4] = { -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };
    lfTCAModel tcam = LF_TCA_MODEL_NONE;

    memset (&spline, 0, sizeof (spline));
    for (int i = 0; CalibTCA [i]; i++)
    {
        lfLensCalibTCA *c = CalibTCA [i];
        if (c->Model == LF_TCA_MODEL_NONE)
            continue;

        // Take into account just the first encountered lens model
        if (tcam == LF_TCA_MODEL_NONE)
            tcam = c->Model;
        else if (tcam != c->Model)
        {
            continue;
        }

        float df = focal - c->Focal;
        if (df == 0.0)
        {
            // Exact match found, don't care to interpolate
            res = *c;
            return true;
        }

        __insert_spline (spline_ptr, spline_dist, df, c);
    }

    if (!spline [1] || !spline [2])
    {
        if (spline [1])
            res = *spline [1];
        else if (spline [2])
            res = *spline [2];
        else
            return false;

        return true;
    }

    // No exact match found, interpolate the model parameters
    res.Model = tcam;
    res.Focal = focal;

    float t = (focal - spline [1]->Focal) / (spline [2]->Focal - spline [1]->Focal);

    for (size_t i = 0; i < ARRAY_LEN (res.Terms); i++)
    {
        float values [5] = {spline [0] ? spline [0]->Focal : NAN, spline [1]->Focal,
                            spline [2]->Focal, spline [3] ? spline [3]->Focal : NAN,
                            focal};
        __parameter_scales (values, 5, LF_MODIFY_TCA, tcam, i);
        res.Terms [i] = _lf_interpolate (
            spline [0] ? spline [0]->Terms [i] * values [0] : FLT_MAX,
            spline [1]->Terms [i] * values [1], spline [2]->Terms [i] * values [2],
            spline [3] ? spline [3]->Terms [i] * values [3] : FLT_MAX,
            t) / values [4];
    }

    return true;
}

static float __vignetting_dist (
    const lfLens *l, const lfLensCalibVignetting &x, float focal, float aperture, float distance)
{
    // translate every value to linear scale and normalize
    // approximatively to range 0..1
    float f1 = focal - l->MinFocal;
    float f2 = x.Focal - l->MinFocal;
    float df = l->MaxFocal - l->MinFocal;
    if (df != 0)
    {
        f1 /= df;
        f2 /= df;
    }
    float a1 = 4.0 / aperture;
    float a2 = 4.0 / x.Aperture;
    float d1 = 0.1 / distance;
    float d2 = 0.1 / x.Distance;

    return sqrt (square (f2 - f1) + square (a2 - a1) + square (d2 - d1));
}

bool lfLens::InterpolateVignetting (
    float focal, float aperture, float distance, lfLensCalibVignetting &res) const
{
    if (!CalibVignetting)
        return false;

    lfVignettingModel vm = LF_VIGNETTING_MODEL_NONE;
    res.Focal = focal;
    res.Aperture = aperture;
    res.Distance = distance;
    for (size_t i = 0; i < ARRAY_LEN (res.Terms); i++)
        res.Terms [i] = 0;

    // Use http://en.wikipedia.org/wiki/Inverse_distance_weighting with
    // p = 3.5.
    float total_weighting = 0;
    const float power = 3.5;

    float smallest_interpolation_distance = FLT_MAX;
    for (int i = 0; CalibVignetting [i]; i++)
    {
        const lfLensCalibVignetting* c = CalibVignetting [i];
        // Take into account just the first encountered lens model
        if (vm == LF_VIGNETTING_MODEL_NONE)
        {
            vm = c->Model;
            res.Model = vm;
        } 
        else if (vm != c->Model)
        {
            continue;
        }

        float interpolation_distance = __vignetting_dist (this, *c, focal, aperture, distance);
        if (interpolation_distance < 0.0001) {
            res = *c;
            return true;
        }

        smallest_interpolation_distance = std::min(smallest_interpolation_distance, interpolation_distance);
        float weighting = fabs (1.0 / pow (interpolation_distance, power));
        for (size_t i = 0; i < ARRAY_LEN (res.Terms); i++)
        {
            float values [1] = {c->Focal};
            __parameter_scales (values, 1, LF_MODIFY_VIGNETTING, vm, i);
            res.Terms [i] += weighting * c->Terms [i] * values [0];
        }
        total_weighting += weighting;
    }
    
    if (smallest_interpolation_distance > 1)
        return false;
    
    if (total_weighting > 0 && smallest_interpolation_distance < FLT_MAX)
    {
        for (size_t i = 0; i < ARRAY_LEN (res.Terms); i++)
        {
            float values [1] = {focal};
            __parameter_scales (values, 1, LF_MODIFY_VIGNETTING, vm, i);
            res.Terms [i] /= total_weighting * values [0];
        }
        return true;
    } else 
        return false;
}

bool lfLens::InterpolateCrop (float focal, lfLensCalibCrop &res) const
{
    if (!CalibCrop)
        return false;

    union
    {
        lfLensCalibCrop *spline [4];
        void *spline_ptr [4];
    };
    float spline_dist [4] = { -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };
    lfCropMode cm = LF_NO_CROP;

    memset (spline, 0, sizeof (spline));
    for (int i = 0; CalibCrop [i]; i++)
    {
        lfLensCalibCrop *c = CalibCrop [i];
        if (c->CropMode == LF_NO_CROP)
            continue;

        // Take into account just the first encountered crop mode
        if (cm == LF_NO_CROP)
            cm = c->CropMode;
        else if (cm != c->CropMode)
        {
            continue;
        }

        float df = focal - c->Focal;
        if (df == 0.0)
        {
            // Exact match found, don't care to interpolate
            res = *c;
            return true;
        }

        __insert_spline (spline_ptr, spline_dist, df, c);
    }

    if (!spline [1] || !spline [2])
    {
        if (spline [1])
            res = *spline [1];
        else if (spline [2])
            res = *spline [2];
        else
            return false;

        return true;
    }

    // No exact match found, interpolate the model parameters
    res.CropMode = cm;
    res.Focal = focal;

    float t = (focal - spline [1]->Focal) / (spline [2]->Focal - spline [1]->Focal);

    for (size_t i = 0; i < ARRAY_LEN (res.Crop); i++)
        res.Crop [i] = _lf_interpolate (
            spline [0] ? spline [0]->Crop [i] : FLT_MAX,
            spline [1]->Crop [i], spline [2]->Crop [i],
            spline [3] ? spline [3]->Crop [i] : FLT_MAX, t);

    return true;
}

bool lfLens::InterpolateFov (float focal, lfLensCalibFov &res) const
{
    if (!CalibFov)
        return false;

    union
    {
        lfLensCalibFov *spline [4];
        void *spline_ptr [4];
    };
    float spline_dist [4] = { -FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX };

    memset (spline, 0, sizeof (spline));
    int counter=0;
    for (int i = 0; CalibFov [i]; i++)
    {
        lfLensCalibFov *c = CalibFov [i];
        if (c->FieldOfView == 0)
            continue;

        counter++;
        float df = focal - c->Focal;
        if (df == 0.0)
        {
            // Exact match found, don't care to interpolate
            res = *c;
            return true;
        }

        __insert_spline (spline_ptr, spline_dist, df, c);
    }

    //no valid data found
    if (counter==0)
        return false;

    if (!spline [1] || !spline [2])
    {
        if (spline [1])
            res = *spline [1];
        else if (spline [2])
            res = *spline [2];
        else
            return false;

        return true;
    }

    // No exact match found, interpolate the model parameters
    res.Focal = focal;

    float t = (focal - spline [1]->Focal) / (spline [2]->Focal - spline [1]->Focal);

    res.FieldOfView = _lf_interpolate (
        spline [0] ? spline [0]->FieldOfView : FLT_MAX,
        spline [1]->FieldOfView, spline [2]->FieldOfView,
        spline [3] ? spline [3]->FieldOfView : FLT_MAX, t);

    return true;
}

int _lf_lens_parameters_compare (const lfLens *i1, const lfLens *i2)
{
    int cmp = int ((i1->MinFocal - i2->MinFocal) * 100);
    if (cmp != 0)
        return cmp;

    cmp = int ((i1->MaxFocal - i2->MaxFocal) * 100);
    if (cmp != 0)
        return cmp;

    return int ((i1->MinAperture - i2->MinAperture) * 100);

    // MaxAperture is usually not given in database...
    // so it's a guessed value, often incorrect.
}

static int _lf_compare_num (float a, float b)
{
    if (!a || !b)
        return 0; // neutral

    float r = a / b;
    if (r <= 0.99 || r >= 1.01)
        return -1; // strong no
    return +1; // strong yes
}

// int _lf_lens_compare_score (const lfLens *pattern, const lfLens *match,
//                             lfFuzzyStrCmp *fuzzycmp, const char **compat_mounts)
// {
//     int score = 0;

//     // Compare numeric fields first since that's easy.

//     if (pattern->Type != LF_UNKNOWN)
//         if (pattern->Type != match->Type)
//             return 0;

//     if (pattern->CropFactor > 0.01 && pattern->CropFactor < match->CropFactor * 0.96)
//         return 0;

//     if (pattern->CropFactor >= match->CropFactor * 1.41)
//         score += 2;
//     else if (pattern->CropFactor >= match->CropFactor * 1.31)
//         score += 4;
//     else if (pattern->CropFactor >= match->CropFactor * 1.21)
//         score += 6;
//     else if (pattern->CropFactor >= match->CropFactor * 1.11)
//         score += 8;
//     else if (pattern->CropFactor >= match->CropFactor * 1.01)
//         score += 10;
//     else if (pattern->CropFactor >= match->CropFactor)
//         score += 5;
//     else if (pattern->CropFactor >= match->CropFactor * 0.96)
//         score += 3;

//     switch (_lf_compare_num (pattern->MinFocal, match->MinFocal))
//     {
//         case -1:
//             return 0;

//         case +1:
//             score += 10;
//             break;
//     }

//     switch (_lf_compare_num (pattern->MaxFocal, match->MaxFocal))
//     {
//         case -1:
//             return 0;

//         case +1:
//             score += 10;
//             break;
//     }

//     switch (_lf_compare_num (pattern->MinAperture, match->MinAperture))
//     {
//         case -1:
//             return 0;

//         case +1:
//             score += 10;
//             break;
//     }

//     switch (_lf_compare_num (pattern->MaxAperture, match->MaxAperture))
//     {
//         case -1:
//             return 0;

//         case +1:
//             score += 10;
//             break;
//     }

//     switch (_lf_compare_num (pattern->AspectRatio, match->AspectRatio))
//     {
//         case -1:
//             return 0;

//         case +1:
//             score += 10;
//             break;
//     }

//     if (compat_mounts && !compat_mounts [0])
//         compat_mounts = NULL;

//     // Check the lens mount, if specified
//     if (match->Mounts && (pattern->Mounts || compat_mounts))
//     {
//         bool matching_mount_found = false;

//         if (pattern->Mounts)
//             for (int i = 0; pattern->Mounts [i]; i++)
//                 for (int j = 0; match->Mounts [j]; j++)
//                     if (!_lf_strcmp (pattern->Mounts [i], match->Mounts [j]))
//                     {
//                         matching_mount_found = true;
//                         score += 10;
//                         goto exit_mount_search;
//                     }

//         if (compat_mounts)
//             for (int i = 0; compat_mounts [i]; i++)
//                 for (int j = 0; match->Mounts [j]; j++)
//                     if (!_lf_strcmp (compat_mounts [i], match->Mounts [j]))
//                     {
//                         matching_mount_found = true;
//                         score += 9;
//                         goto exit_mount_search;
//                     }

//     exit_mount_search:
//         if (!matching_mount_found)
//             return 0;
//     }

//     // If maker is specified, check it using our patented _lf_strcmp(tm) technology
//     if (pattern->Maker && match->Maker)
//     {
//         if (_lf_mlstrcmp (pattern->Maker, match->Maker) != 0)
//             return 0; // Bah! different maker.
//         else
//             score += 10; // Good doggy, here's a cookie
//     }

//     // And now the most complex part - compare models
//     if (pattern->Model && match->Model)
//     {
//         int _score = fuzzycmp->Compare (match->Model);
//         if (!_score)
//             return 0; // Model does not match
//         _score = (_score * 4) / 10;
//         if (!_score)
//             _score = 1;
//         score += _score;
//     }

//     return score;
// }

//---------------------------// The C interface //---------------------------//

lfLens *lf_lens_new ()
{
    return new lfLens ();
}

void lf_lens_destroy (lfLens *lens)
{
    delete lens;
}

void lf_lens_copy (lfLens *dest, const lfLens *source)
{
    *dest = *source;
}

void lf_lens_guess_parameters (lfLens *lens)
{
    lens->GuessParameters ();
}

cbool lf_lens_check (lfLens *lens)
{
    return lens->Check ();
}

const char *lf_get_distortion_model_desc (
    enum lfDistortionModel model, const char **details, const lfParameter ***params)
{
    return lfLens::GetDistortionModelDesc (model, details, params);
}

const char *lf_get_tca_model_desc (
    enum lfTCAModel model, const char **details, const lfParameter ***params)
{
    return lfLens::GetTCAModelDesc (model, details, params);
}

const char *lf_get_vignetting_model_desc (
    enum lfVignettingModel model, const char **details, const lfParameter ***params)
{
    return lfLens::GetVignettingModelDesc (model, details, params);
}

const char *lf_get_crop_desc (
    enum lfCropMode mode, const char **details, const lfParameter ***params)
{
    return lfLens::GetCropDesc (mode, details, params);
}

const char *lf_get_lens_type_desc (enum lfLensType type, const char **details)
{
    return lfLens::GetLensTypeDesc (type, details);
}

cbool lf_lens_interpolate_distortion (const lfLens *lens, float focal,
    lfLensCalibDistortion *res)
{
    return lens->InterpolateDistortion (focal, *res);
}

cbool lf_lens_interpolate_tca (const lfLens *lens, float focal, lfLensCalibTCA *res)
{
    return lens->InterpolateTCA (focal, *res);
}

cbool lf_lens_interpolate_vignetting (const lfLens *lens, float focal, float aperture,
    float distance, lfLensCalibVignetting *res)
{
    return lens->InterpolateVignetting (focal, aperture, distance, *res);
}

cbool lf_lens_interpolate_crop (const lfLens *lens, float focal,
    lfLensCalibCrop *res)
{
    return lens->InterpolateCrop (focal, *res);
}

cbool lf_lens_interpolate_fov (const lfLens *lens, float focal,
    lfLensCalibFov *res)
{
    return lens->InterpolateFov (focal, *res);
}

void lf_lens_add_calib_distortion (lfLens *lens, const lfLensCalibDistortion *dc)
{
    lens->AddCalibDistortion (dc);
}

cbool lf_lens_remove_calib_distortion (lfLens *lens, int idx)
{
    return lens->RemoveCalibDistortion (idx);
}

void lf_lens_add_calib_tca (lfLens *lens, const lfLensCalibTCA *tcac)
{
    lens->AddCalibTCA (tcac);
}

cbool lf_lens_remove_calib_tca (lfLens *lens, int idx)
{
    return lens->RemoveCalibTCA (idx);
}

void lf_lens_add_calib_vignetting (lfLens *lens, const lfLensCalibVignetting *vc)
{
    lens->AddCalibVignetting (vc);
}

cbool lf_lens_remove_calib_vignetting (lfLens *lens, int idx)
{
    return lens->RemoveCalibVignetting (idx);
}

void lf_lens_add_calib_crop (lfLens *lens, const lfLensCalibCrop *lcc)
{
    lens->AddCalibCrop (lcc);
}

cbool lf_lens_remove_calib_crop (lfLens *lens, int idx)
{
    return lens->RemoveCalibCrop (idx);
}


void lf_lens_add_calib_fov (lfLens *lens, const lfLensCalibFov *lcf)
{
    lens->AddCalibFov (lcf);
}

cbool lf_lens_remove_calib_fov (lfLens *lens, int idx)
{
    return lens->RemoveCalibFov (idx);
}
