/*
    Lensfun - a library for maintaining a database of photographical lenses,
    and providing the means to correct some of the typical lens distortions.
    Copyright (C) 2007 by Andrew Zabolotny

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __LENSFUN_H__
#define __LENSFUN_H__

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus

extern "C" {
/** Helper macro to make C/C++ work similarly */
#  define C_TYPEDEF(t,c)
#else
#  define C_TYPEDEF(t,c) typedef t c c;
#endif

/**
 * @file lensfun.h
 * This file defines the interface to the Lensfun library.
 */

/*----------------------------------------------------------------------------*/

/**
 * @defgroup Auxiliary Auxiliary definitions and functions
 * @brief These functions will help handling basic structures of the library.
 * @{
 */

/// Major library version number
#define LF_VERSION_MAJOR	0
/// Minor library version number
#define LF_VERSION_MINOR	3
/// Library micro version number
#define LF_VERSION_MICRO	2
/// Library bugfix number
#define LF_VERSION_BUGFIX	0
/// Full library version
#define LF_VERSION	((LF_VERSION_MAJOR << 24) | (LF_VERSION_MINOR << 16) | (LF_VERSION_MICRO << 8) | LF_VERSION_BUGFIX)

/// Oldest database version supported by this release
#define LF_MIN_DATABASE_VERSION	0
/// Latest database version supported by this release
#define LF_MAX_DATABASE_VERSION	2

#define LF_EXPORT    __declspec(dllexport)

/// C-compatible bool type; don't bother to define Yet Another Boolean Type
#define cbool int

/**
 * The storage of "multi-language" strings is simple yet flexible,
 * handy and effective. The first (default) string comes first, terminated
 * by \\0 as usual, after that a language code follows, then \\0 again,
 * then the translated value and so on. The list terminates as soon as
 * a \\0 is encountered instead of next string, e.g. last string in list
 * is terminated with two null characters.
 */
typedef char *lfMLstr;

/** liblensfun error codes: negative codes are -errno, positive are here */
enum lfError
{
    /** No error occured */
    LF_NO_ERROR = 0,
    /** Wrong XML data format */
    LF_WRONG_FORMAT,
    /** No database could be loaded */
    LF_NO_DATABASE
};

C_TYPEDEF (enum, lfError)

/** The type of a 8-bit pixel */
typedef unsigned char lf_u8;
/** The type of a 16-bit pixel */
typedef unsigned short lf_u16;
/** The type of a 32-bit pixel */
typedef unsigned int lf_u32;
/** The type of a 32-bit floating-point pixel */
typedef float lf_f32;
/** The type of a 64-bit floating-point pixel */
typedef double lf_f64;

/**
 * The basics of memory allocation: never free objects allocated by the
 * library yourselves, instead use this function. It is a direct
 * equivalent of standard C free(), however you should not use free()
 * in the event that the library uses a separate heap.
 * @param data
 *     A pointer to memory to be freed.
 */
LF_EXPORT void lf_free (void *data);

/**
 * @brief Get a string corresponding to current locale from a multi-language
 * string.
 *
 * Current locale is determined from LC_MESSAGES category at the time of
 * the call, e.g. if you change LC_MESSAGES at runtime, next calls to
 * lf_mlstr_get() will return the string for the new locale.
 */
LF_EXPORT const char *lf_mlstr_get (const lfMLstr str);

/**
 * @brief Add a new translated string to a multi-language string.
 *
 * This uses realloc() so returned value may differ from input.
 * @param str
 *     The string to append to. Can be NULL.
 * @param lang
 *     The language for the new added string. If NULL, the default
 *     string is replaced (the first one in list, without a language
 *     designator).
 * @param trstr
 *     The translated string
 * @return
 *     The reallocated multi-language string. To free a multi-language
 *     string, use lf_free().
 */
LF_EXPORT lfMLstr lf_mlstr_add (lfMLstr str, const char *lang, const char *trstr);

/**
 * @brief Create a complete copy of a multi-language string.
 *
 * @param str
 *     The string to create a copy of
 * @return
 *     A new allocated multi-language string
 */
LF_EXPORT lfMLstr lf_mlstr_dup (const lfMLstr str);

/** @} */

/*----------------------------------------------------------------------------*/

/**
 * @defgroup Mount Structures and functions for camera mounts
 * @brief These structures and functions allow to define and examine
 * the properties of camera mounts.
 * @{
 */

/**
 * @brief This structure contains everything specific to a camera mount.
 *
 * Objects of this type are usually retrieved from the database
 * by using queries (see lfDatabase::FindMount() / lf_db_find_mount()),
 * and can be created manually in which case it is application's
 * responsability to destroy the object when it is not needed anymore.
 */
struct LF_EXPORT lfMount
{
    /** @brief Camera mount name.
     *
     * Mount names for fixed-lens cameras -- and only they -- must start with a
     * lower case letter.
     */
    lfMLstr Name;
    /** A list of compatible mounts */
    char **Compat;

#ifdef __cplusplus
    /**
     * @brief Initialize a new mount object. All fields are set to 0.
     */
    lfMount ();

    /**
     * Copy constructor
     */
    lfMount (const lfMount &other);

    /**
     * Assignment operator
     */
    lfMount &operator = (const lfMount &other);

    /**
     * @brief Destroy a mount object. All allocated fields are freed.
     */
    ~lfMount ();

    /**
     * @brief Add a string to mount name.
     *
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Name field.
     * @param lang
     *     The language this field is in.
     */
    void SetName (const char *val, const char *lang = NULL);

    /**
     * @brief Add a mount name to the list of compatible mounts.
     * @param val
     *     The identifier of the compatible mount.
     */
    void AddCompat (const char *val);

    /**
     * @brief Check if a mount object is valid.
     * @return
     *     true if required fields are ok.
     */
    bool Check ();
#endif
};

C_TYPEDEF (struct, lfMount)

/**
 * @brief Create a new mount object.
 * @return
 *     A new empty mount object.
 * @sa
 *     lfMount::lfMount()
 */
LF_EXPORT lfMount *lf_mount_new ();

/**
 * @brief Destroy a lfMount object.
 * 
 * This is equivalent to C++ "delete mount".
 * @param mount
 *     The mount object to destroy.
 * @sa
 *     lfMount::~lfMount()
 */
LF_EXPORT void lf_mount_destroy (lfMount *mount);

/**
 * @brief Copy the data from one lfMount structure into another.
 * @param dest
 *     The destination object
 * @param source
 *     The source object
 * @sa
 *     lfMount::operator = (const lfMount &)
 */
LF_EXPORT void lf_mount_copy (lfMount *dest, const lfMount *source);

/** @sa lfMount::Check */
LF_EXPORT cbool lf_mount_check (lfMount *mount);

/** @} */

/*----------------------------------------------------------------------------*/

/**
 * @defgroup Camera Structures and functions for cameras
 * @brief These structures and functions allow to define and examine
 * the properties of a camera model.
 * @{
 */

/**
 * @brief Camera data.  Unknown fields are set to NULL.
 *
 * The Maker and Model fields must be filled EXACTLY as they appear in the EXIF
 * data, since this is the only means to detect camera automatically
 * (upper/lowercase is not important, though).  Some different cameras
 * (e.g. Sony Cybershot) are using same EXIF id info for different models, in
 * which case the Variant field should contain the exact model name, but, alas,
 * we cannot automatically choose between such "twin" cameras.
 */
struct LF_EXPORT lfCamera
{
    /** @brief Camera maker (ex: "Rollei") -- same as in EXIF */
    lfMLstr Maker;
    /** @brief Model name (ex: "Rolleiflex SL35") -- same as in EXIF */
    lfMLstr Model;
    /** @brief Camera variant. Some cameras use same EXIF id for different models */
    lfMLstr Variant;
    /** @brief Camera mount type (ex: "QBM") */
    char *Mount;
    /** @brief Camera crop factor (ex: 1.0). Must be defined. */
    float CropFactor;
    /** @brief Camera matching score, used while searching: not actually a camera parameter */
    int Score;

#ifdef __cplusplus
    /**
     * @brief Initialize a new camera object. All fields are set to 0.
     */
    lfCamera ();

    /**
     * Copy constructor.
     */
    lfCamera (const lfCamera &other);

    /**
     * @brief Destroy a camera object. All allocated fields are freed.
     */
    ~lfCamera ();

    /**
     * Assignment operator
     */
    lfCamera &operator = (const lfCamera &other);

    /**
     * @brief Add a string to camera maker.
     * 
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Maker field.
     * @param lang
     *     The language this field is in.
     */
    void SetMaker (const char *val, const char *lang = NULL);

    /**
     * @brief Add a string to camera model.
     *
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Model field.
     * @param lang
     *     The language this field is in.
     */
    void SetModel (const char *val, const char *lang = NULL);

    /**
     * @brief Add a string to camera variant.
     *
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Variant field.
     * @param lang
     *     The language this field is in.
     */
    void SetVariant (const char *val, const char *lang = NULL);

    /**
     * @brief Set the value for camera Mount.
     * @param val
     *     The new value for Mount.
     */
    void SetMount (const char *val);

    /**
     * @brief Check if a camera object is valid.
     * @return
     *     true if the required fields are ok.
     */
    bool Check ();
#endif
};

C_TYPEDEF (struct, lfCamera)

/**
 * @brief Create a new camera object.
 * @return
 *     A new empty camera object.
 * @sa
 *     lfCamera::lfCamera
 */
LF_EXPORT lfCamera *lf_camera_new ();

/**
 * @brief Destroy a lfCamera object.
 *
 * This is equivalent to C++ "delete camera".
 * @param camera
 *     The camera object to destroy.
 * @sa
 *     lfCamera::~lfCamera
 */
LF_EXPORT void lf_camera_destroy (lfCamera *camera);

/** @sa lfCamera::Check */
LF_EXPORT cbool lf_camera_check (lfCamera *camera);

/** @} */

/*----------------------------------------------------------------------------*/

/**
 * @defgroup Lens Structures and functions for lenses
 * @brief These structures and functions allow to define and examine
 * the properties of a lens.
 * @{
 */

/**
 * @brief The Lensfun library implements several lens distortion models.
 * This enum lists them.
 *
 * Distortion usually heavily depends on the focal length, but does not depend
 * on the aperture.  In the following, \f$r_d\f$ refers to the distorted radius
 * (normalised distance to image center), and \f$r_u\f$ refers to the
 * undistorted, corrected radius.  See section \ref actualorder for further
 * information.
 *
 * For a popular explanation of lens distortion see
 * http://www.vanwalree.com/optics/distortion.html
 */
enum lfDistortionModel
{
    /** @brief Distortion parameters are unknown */
    LF_DIST_MODEL_NONE,
    /**
     * @brief 3rd order polynomial model, which is a subset of the PTLens
     * model.
     *
     * \f[r_d = r_u \cdot (1 - k_1 + k_1 r_u^2)\f]
     * The corresponding XML attribute is called “k1”.  It defaults to 0.
     */
    LF_DIST_MODEL_POLY3,
    /**
     * @brief 5th order polynomial model.
     *
     * \f[r_d = r_u \cdot (1 + k_1 r_u^2 + k_2 r_u^4)\f]
     * The corresponding XML attributes are called “k1” and “k2”.  They default
     * to 0.
     * Ref: http://www.imatest.com/docs/distortion.html
     */
    LF_DIST_MODEL_POLY5,
    /**
     * @brief PTLens model, which is also used by Hugin.
     *
     * \f[r_d = r_u \cdot (a r_u^3 + b r_u^2 + c r_u + 1 - a - b - c)\f]
     * The corresponding XML attributes are called “a”, “b”, and “c”.  They
     * default to 0.
     */
    LF_DIST_MODEL_PTLENS,
    /**
     * @brief Adobe Camera Model.  The coordinate system is different here.
     * Everything is measured in units of the focal length of the lens.
     *
     * @f[\begin{aligned}
     * x_d &= x_u (1 + k_1 r_u^2 + k_2 r_u^4 + k_3 r_u^6) + 2x_u(k_4y_u + k_5x_u) + k_5 r_u^2, \\
     * y_d &= y_u (1 + k_1 r_u^2 + k_2 r_u^4 + k_3 r_u^6) + 2y_u(k_4y_u + k_5x_u) + k_4 r_u^2,
     * \end{aligned}
     * @f]
     * with \f$r_u^2 = x_u^2 + y_u^2\f$.  The corresponding XML attributes
     * are called “k1”, “k2”, “k3”, “k4”, and “k5”.  They default to 0.  Note
     * that Adobe's fisheye model is also covered by this by setting the lens
     * type to "fisheye" and using only the \f$k_1\f$ and \f$k_2\f$ parameters.
     * Ref:
     * http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf
     */
    LF_DIST_MODEL_ACM,
};

C_TYPEDEF (enum, lfDistortionModel)

/**
 * @brief Lens distortion calibration data.

 * Lens distortion depends only of focal length. The library will interpolate
 * the coefficients values if data for the exact focal length is not available.
 */
struct lfLensCalibDistortion
{
    /** @brief The type of distortion model used */
    enum lfDistortionModel Model;
    /** @brief Nominal focal length in mm at which this calibration data was
     * taken */
    float Focal;
    /** @brief Real focal length in mm for this nominal focal length
     *
     * When Lensfun speaks of “focal length”, the *nominal* focal length from
     * the EXIF data or the gravure on the lens barrel is meant.  However,
     * especially for fisheye lenses, the real focal length generally differs
     * from that nominal focal length.  With “real focal length”, Lensfun means
     * the focal length in the paraxial approximation, see
     * <http://en.wikipedia.org/wiki/Paraxial_approximation>.  Note that Hugin
     * (as of 2014) implements the calculation of the real focal length
     * wrongly, see <http://article.gmane.org/gmane.comp.misc.ptx/34865>.
     *
     * The default value usually is the nominal focal length.  Only for
     * Hugin-based distortion models, the default is the nominal focal length
     * multiplied by the "d" parameter (i.e. linear in r) in the Hugin
     * polynomial.  This is because it is this focal length that Hugin assumes
     * in its fit.
     */
    float RealFocal;
    /** @brief Whether the real focal length was actually measured.
     */
    cbool RealFocalMeasured;
    /** @brief Distortion coefficients, dependent on model (a,b,c or k1 or k1,k2) */
    float Terms [5];
};

C_TYPEDEF (struct, lfLensCalibDistortion)

/**
 * @brief The Lensfun library supports several models for lens lateral
 * chromatic aberrations (also called transversal chromatic
 * aberrations, TCA).
 *
 * TCAs depend on focal length, but does not depend of the aperture.  In the
 * following, \f$r_d\f$ refers to the distorted radius (normalised distance to
 * image center), and \f$r_u\f$ refers to the undistorted, corrected radius.
 * See section \ref actualorder for further information.
 *
 * For a popular explanation of chromatic aberrations see
 * http://www.vanwalree.com/optics/chromatic.html
 */
enum lfTCAModel
{
    /** @brief No TCA correction data is known */
    LF_TCA_MODEL_NONE,
    /**
     * @brief Linear lateral chromatic aberrations model.
     *
     * \f[\begin{aligned}
     * r_{d,R} &= r_{u,R} k_R \\
     * r_{d,B} &= r_{u,B} k_B
     * \end{aligned}\f]
     * The corresponding XML attributes are called “kr” and “kb”.  They default
     * to 1.
     * Ref: http://cipa.icomos.org/fileadmin/template/doc/TURIN/403.pdf
     */
    LF_TCA_MODEL_LINEAR,
    /**
     * @brief Third order polynomial.
     *
     * \f[\begin{aligned}
     * r_{d,R} &= r_{u,R} \cdot (b_R r_{u,R}^2 + c_R r_{u,R} + v_R) \\
     * r_{d,B} &= r_{u,B} \cdot (b_B r_{u,B}^2 + c_B r_{u,B} + v_B)
     * \end{aligned}\f]
     * The corresponding XML attributes are called “br”, “cr”,
     * “vr”, “bb”, “cb”, and “vb”.  vr and vb default to 1, the rest to 0.
     * Ref: http://wiki.panotools.org/Tca_correct
     */
    LF_TCA_MODEL_POLY3,
    /**
     * @brief Adobe camera model for TCA.  The coordinate system is different
     * here.  Everything is measured in units of the focal length of the lens.
     *
     * \f[\begin{aligned}
     * x_{d,R} &= \alpha_0 \left(\left(1 + \alpha_1 r_{u,R}^2 +
                  \alpha_2 r_{u,R}^4 + \alpha_3 r_{u,R}^6\right) x_{u,R} +
                  2(\alpha_4 y_{u,R} + \alpha_5 x_{u,R}) x_{u,R} +
                  \alpha_5 r_{u,R}^2\right), \\
     * y_{d,R} &= \alpha_0 \left(\left(1 + \alpha_1 r_{u,R}^2 +
                  \alpha_2 r_{u,R}^4 + \alpha_3 r_{u,R}^6\right) y_{u,R} +
                  2(\alpha_4 y_{u,R} + \alpha_5 x_{u,R}) y_{u,R} +
                  \alpha_4 r_{u,R}^2\right), \\
     * x_{d,B} &= \beta_0 \left(\left(1 + \beta_1 r_{u,B}^2 +
                  \beta_2 r_{u,B}^4 + \beta_3 r_{u,B}^6\right) x_{u,B} +
                  2(\beta_4 y_{u,B} + \beta_5 x_{u,B}) x_{u,B} +
                  \beta_5 r_{u,B}^2\right), \\
     * y_{d,B} &= \beta_0 \left(\left(1 + \beta_1 r_{u,B}^2 +
                  \beta_2 r_{u,B}^4 + \beta_3 r_{u,B}^6\right) y_{u,B} +
                  2(\beta_4 y_{u,B} + \beta_5 x_{u,B}) y_{u,B} +
                  \beta_4 r_{u,B}^2\right),
     * \end{aligned}\f]
     * with \f$r_{u,\ast} = \sqrt{x_{u,\ast}^2 + y_{u,\ast}^2}\f$.  The
     * corresponding XML attributes are called “alpha0” to “alpha5”, and
     * “beta0” to “beta5”.  alpha0 and beta0 default to 1, the rest to 0.  Ref:
     * http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf
     */
    LF_TCA_MODEL_ACM
};

C_TYPEDEF (enum, lfTCAModel)

/**
 * @brief Laterlal chromatic aberrations calibration data.
 *
 * Chromatic aberrations depend on focal length and aperture value. The library
 * will interpolate the coefficients if data for the exact focal length and
 * aperture value is not available with priority for a more exact focal length.
 */
struct lfLensCalibTCA
{
    /** @brief The lateral chromatic aberration model used */
    enum lfTCAModel Model;
    /** @brief Focal length in mm at which this calibration data was taken */
    float Focal;
    /** @brief The coefficients for TCA, dependent on model; separate for R and B */
    float Terms [12];
};

C_TYPEDEF (struct, lfLensCalibTCA)

/**
 * @brief The Lensfun library supports several models for lens vignetting
 * correction.
 *
 * We focus on optical and natural vignetting since they can be generalized for
 * all lenses of a certain type; mechanical vignetting is out of the scope of
 * this library.
 *
 * Vignetting is dependent on focal length and aperture.  There is
 * also a slight dependence on focus distance.  In the following,
 * \f$C_d\f$ refers to the corrected destination image pixel brightness, and
 * \f$C_s\f$ refers to the uncorrected source image pixel brightness.
 *
 * For a popular explanation of vignetting see
 * http://www.vanwalree.com/optics/vignetting.html
 */
enum lfVignettingModel
{
    /** @brief No vignetting correction data is known */
    LF_VIGNETTING_MODEL_NONE,
    /**
     * @brief Pablo D'Angelo vignetting model
     * (which is a more general variant of the \f$\cos^4\f$ law).
     *
     * \f[C_d = C_s / (1 + k_1 r^2 + k_2 r^4 + k_3 r^6)\f]
     * The corresponding XML attributes are called “k1”, “k2”, and “k3”.  They
     * default to 0.
     * Ref: http://hugin.sourceforge.net/tech/
     */
    LF_VIGNETTING_MODEL_PA,
    /**
     * @brief Adobe's vignetting model.  It differs from D'Angelo's model only
     * in the coordinate system.  Everything is measured in units of the focal
     * length of the lens.
     *
     * \f[C_d = C_s \cdot (1 + \alpha_1 r^2 + \alpha_2 r^4 + \alpha_3 r^6).\f]
     * The corresponding XML attributes are called “alpha1”, “alpha2”, and
     * “alpha3”.  They default to 0.  Ref:
     * http://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf
     */
    LF_VIGNETTING_MODEL_ACM
};

C_TYPEDEF (enum, lfVignettingModel)

/**
 * @brief Lens vignetting calibration data.
 *
 * Vignetting depends on focal length, aperture and focus distance. The library
 * will interpolate the coefficients if data for the exact focal length,
 * aperture, and focus distance is not available.
 */
struct lfLensCalibVignetting
{
    /** @brief The lens vignetting model used */
    enum lfVignettingModel Model;
    /** @brief Focal length in mm at which this calibration data was taken */
    float Focal;
    /** @brief Aperture (f-number) at which this calibration data was taken */
    float Aperture;
    /** @brief Focus distance in meters */
    float Distance;
    /** @brief Lens vignetting model coefficients (depending on model) */
    float Terms [3];
};

C_TYPEDEF (struct, lfLensCalibVignetting)

/**
 *  @brief Different crop modes
 */
enum lfCropMode
{
    /** @brief no crop at all */
    LF_NO_CROP,
    /** @brief use a rectangular crop */
    LF_CROP_RECTANGLE,
    /** @brief use a circular crop, e.g. for circular fisheye images */
    LF_CROP_CIRCLE
};

C_TYPEDEF(enum, lfCropMode)

/**
 *  @brief Struct to save image crop, which can depend on the focal length
 */
struct lfLensCalibCrop
{
    /** @brief Focal length in mm at which this calibration data was taken */
    float Focal;
    /** @brief crop mode which should be applied to image to get rid of black borders */
    enum lfCropMode CropMode;
    /** @brief Crop coordinates, relative to image corresponding image dimension 
     *
     *  Crop goes left - 0, right - 1, top - 2, bottom - 3 
     *  Left/right refers always to long side (width in landscape mode), 
     *  top bottom to short side (height in landscape).
     *  Also negative values are allowed for cropping of fisheye images,
     *  where the crop circle can extend above the image border.
     */
    float Crop [4];
};

C_TYPEDEF (struct, lfLensCalibCrop)

/**
 *  @brief Struct to save calibrated field of view, which can depends on the focal length (DEPRECATED)
 *
 *  The Field of View (FOV) database entry is deprecated since Lensfun 
 *  version 0.3 and will be removed in future releases.
 *
 */
struct lfLensCalibFov
{
    /** Focal length in mm at which this calibration data was taken */
    float Focal;
    /** @brief Field of view for given images
     * 
     *  The Field of View (FOV) database entry is deprecated since Lensfun 
     *  version 0.3 and will be removed in future releases.
     *
     *  Especially for fisheye images the field of view calculated from the (EXIF) focal
     *  length differs slightly from the real field of view. The real field of view can be 
     *  stored in this field 
     */
    float FieldOfView;
};

C_TYPEDEF (struct, lfLensCalibFov)

/**
 * @brief This structure describes a single parameter for some lens model.
 */
struct lfParameter
{
    /** @brief Parameter name (something like 'k', 'k3', 'omega' etc.) */
    const char *Name;
    /** @brief Minimal value that has sense */
    float Min;
    /** @brief Maximal value that has sense */
    float Max;
    /** @brief Default value for the parameter */
    float Default;
};

C_TYPEDEF (struct, lfParameter)

/**
 * @brief Lens type.  See \ref changeofprojection for further information.
 */
enum lfLensType
{
    /** @brief Unknown lens type */
    LF_UNKNOWN,
    /** @brief Rectilinear lens
     *
     * Straight lines remain stright; 99% of all lenses are of this type.
     */
    LF_RECTILINEAR,
    /**
     * @brief Equidistant fisheye
     *
     * Ref: http://wiki.panotools.org/Fisheye_Projection
     */
    LF_FISHEYE,
    /**
     * @brief Panoramic (cylindrical)
     *
     * Not that there are such lenses, but useful to convert images \a to this
     * type, especially fish-eye images.
     */
    LF_PANORAMIC,
    /**
     * @brief Equirectangular
     *
     * Not that there are such lenses, but useful to convert images \a to this
     * type, especially fish-eye images.
     */
    LF_EQUIRECTANGULAR,
    /** @brief Orthographic fisheye */
    LF_FISHEYE_ORTHOGRAPHIC,
    /** @brief Stereographic fisheye */
    LF_FISHEYE_STEREOGRAPHIC,
    /** @brief Equisolid fisheye */
    LF_FISHEYE_EQUISOLID,
    /**
     * @brief Fisheye as measured by Thoby (for Nikkor 10.5).
     *
     * Ref: http://michel.thoby.free.fr/Blur_Panorama/Nikkor10-5mm_or_Sigma8mm/Sigma_or_Nikkor/Comparison_Short_Version_Eng.html
     */
    LF_FISHEYE_THOBY
};

C_TYPEDEF (enum, lfLensType)

/**
 * @brief Lens data.
 * Unknown fields are set to NULL or 0.
 *
 * To manually create a new lens object, fill the fields for which you 
 * have data, and invoke the lfLens::Check() or lf_lens_check() function, 
 * which will check if existing data is enough and will automatically fill
 * some fields using information extracted from lens name.
 */
struct LF_EXPORT lfLens
{
    /** Lens maker (ex: "Rollei") */
    lfMLstr Maker;
    /** Lens model (ex: "Zoom-Rolleinar") */
    lfMLstr Model;
    /** Minimum focal length, mm (ex: 35). */
    float MinFocal;
    /** Maximum focal length, mm (ex: 105). Can be equal to MinFocal. */
    float MaxFocal;
    /** Smallest f-number possible (ex: 3.5). */
    float MinAperture;
    /** Biggest f-number possible (ex: 22). */
    float MaxAperture;
    /** Available mounts (NULL-terminated list) (ex: { "QBM", NULL }) */
    char **Mounts;
    /**
     * The horizontal shift of all lens distortions.
     * Note that distortion and TCA uses same geometrical lens center.
     * It is given as a relative value to avoide dependency on the
     * image and/or sensor sizes. The calibrated delta X and Y values are
     * numbers in the -0.5 .. +0.5 range. For 1 we take the maximal image
     * dimension (width or height) - this is related to the fact that the
     * lens has a circular field of projection disregarding sensor size.
     */
    float CenterX;
    /** The vertical shift of all lens distortions. (0,0) for geometric center */
    float CenterY;
    /** Crop factor at which calibration measurements were taken.  Must be defined. */
    float CropFactor;
    /** Aspect ratio of the images used for calibration measurements. */
    float AspectRatio;
    /** Lens type */
    lfLensType Type;
    /** Lens distortion calibration data, NULL-terminated (unsorted) */
    lfLensCalibDistortion **CalibDistortion;
    /** Lens TCA calibration data, NULL-terminated (unsorted) */
    lfLensCalibTCA **CalibTCA;
    /** Lens vignetting calibration data, NULL-terminated (unsorted) */
    lfLensCalibVignetting **CalibVignetting;
    /** Crop data, NULL-terminated (unsorted) */
    lfLensCalibCrop **CalibCrop;
    /** Field of view calibration data, NULL-terminated (unsorted) */
    lfLensCalibFov **CalibFov;
    /** Lens matching score, used while searching: not actually a lens parameter */
    int Score;

#ifdef __cplusplus
    /**
     * @brief Create a new lens object, initializing all fields to default values.
     */
    lfLens ();

    /**
     * Copy constructor.
     */
    lfLens (const lfLens &other);

    /**
     * @brief Destroy this and all associated objects.
     */
    ~lfLens ();

    /**
     * Assignment operator
     */
    lfLens &operator = (const lfLens &other);

    /**
     * @brief Add a string to camera maker.
     *
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Maker field.
     * @param lang
     *     The language this field is in.
     */
    void SetMaker (const char *val, const char *lang = NULL);

    /**
     * @brief Add a string to camera model.
     *
     * If lang is NULL, this replaces the default value, otherwise a new
     * language value is appended.
     * @param val
     *     The new value for the Model field.
     * @param lang
     *     The language this field is in.
     */
    void SetModel (const char *val, const char *lang = NULL);

    /**
     * @brief Add a new mount type to this lens.
     *
     * This is not a multi-language string, this it's just plain replaced.
     * @param val
     *     The new value to add to the Mounts array.
     */
    void AddMount (const char *val);

    /**
     * @brief Add a new distortion calibration structure to the pool.
     *
     * The objects is copied, thus you can reuse it as soon as
     * this function returns.
     * @param dc
     *     The distortion calibration structure.
     */
    void AddCalibDistortion (const lfLensCalibDistortion *dc);

    /**
     * @brief Remove a calibration entry from the distortion calibration data.
     * @param idx
     *     The calibration data index (zero-based).
     */
    bool RemoveCalibDistortion (int idx);

    /**
     * @brief Add a new transversal chromatic aberration calibration structure
     * to the pool.
     *
     * The objects is copied, thus you can reuse it as soon as
     * this function returns.
     * @param tcac
     *     The transversal chromatic aberration calibration structure.
     */
    void AddCalibTCA (const lfLensCalibTCA *tcac);

    /**
     * @brief Remove a calibration entry from the TCA calibration data.
     * @param idx
     *     The calibration data index (zero-based).
     */
    bool RemoveCalibTCA (int idx);

    /**
     * @brief Add a new vignetting calibration structure to the pool.
     *
     * The objects is copied, thus you can reuse it as soon as
     * this function returns.
     * @param vc
     *     The vignetting calibration structure.
     */
    void AddCalibVignetting (const lfLensCalibVignetting *vc);

    /**
     * @brief Remove a calibration entry from the vignetting calibration data.
     * @param idx
     *     The calibration data index (zero-based).
     */
    bool RemoveCalibVignetting (int idx);

    /**
     * @brief Add a new lens crop structure to the pool.
     *
     * The objects is copied, thus you can reuse it as soon as
     * this function returns.
     * @param cc 
     *     The lens crop structure.
     */
    void AddCalibCrop (const lfLensCalibCrop *cc);

    /**
     * @brief Remove a lens crop entry from the lens crop structure.
     * @param idx
     *     The lens crop data index (zero-based).
     */
    bool RemoveCalibCrop (int idx);

    /**
     * @brief Add a new lens fov structure to the pool. 
     *
     * The Field of View (FOV) database entry is deprecated since Lensfun 
     * version 0.3 and will be removed in future releases.
     *
     * The objects is copied, thus you can reuse it as soon as
     * this function returns.
     * @param cf
     *     The lens fov structure.
     */
    void AddCalibFov (const lfLensCalibFov *cf);

    /**
     * @brief Remove a field of view  entry from the lens fov structure.
     *
     * The Field of View (FOV) database entry is deprecated since Lensfun 
     * version 0.3 and will be removed in future releases.
     *
     * @param idx
     *     The lens information data index (zero-based).
     */
    bool RemoveCalibFov (int idx);

    /**
     * @brief This method fills some fields if they are missing but
     * can be derived from other fields.
     *
     * This includes such non-obvious parameters like the range of focal
     * lengths or the range of apertures, which can be derived from lens named
     * (which is intelligently parsed) or from the list of calibrations.
     */
    void GuessParameters ();

    /**
     * @brief Check if a lens object is valid.
     * @return
     *     true if the required fields are ok.
     */
    bool Check ();

    /**
     * @brief Get the human-readable distortion model name and the descriptions
     * of the parameters required by this model.
     * @param model
     *     The model.
     * @param details
     *     If not NULL, this string will be set to a more detailed (technical)
     *     description of the model. This string may contain newlines.
     * @param params
     *     If not NULL, this variable will be set to a pointer to an array
     *     of lfParameter structures, every structure describes a model
     *     parameter. The list is NULL-terminated.
     * @return
     *     A short name of the distortion model or NULL if model is unknown.
     */
    static const char *GetDistortionModelDesc (
        lfDistortionModel model, const char **details, const lfParameter ***params);
    /**
     * @brief Get the human-readable transversal chromatic aberrations model name
     * and the descriptions of the parameters required by this model.
     * @param model
     *     The model.
     * @param details
     *     If not NULL, this string will be set to a more detailed (technical)
     *     description of the model. This string may contain newlines.
     * @param params
     *     If not NULL, this variable will be set to a pointer to an array
     *     of lfParameter structures, every structure describes a model
     *     parameter. The list is NULL-terminated.
     * @return
     *     A short name of the TCA model or NULL if model is unknown.
     */
    static const char *GetTCAModelDesc (
        lfTCAModel model, const char **details, const lfParameter ***params);

    /**
     * @brief Get the human-readable vignetting model name and the descriptions
     * of the parameters required by this model.
     * @param model
     *     The model.
     * @param details
     *     If not NULL, this string will be set to a more detailed (technical)
     *     description of the model. This string may contain newlines.
     * @param params
     *     If not NULL, this variable will be set to a pointer to an array
     *     of lfParameter structures, every structure describes a model
     *     parameter. The list is NULL-terminated.
     * @return
     *     A short name of the vignetting model or NULL if model is unknown.
     */
    static const char *GetVignettingModelDesc (
        lfVignettingModel model, const char **details, const lfParameter ***params);

    /**
     * @brief Get the human-readable crop name and the descriptions
     * of the parameters required by this model.
     * @param mode
     *     The crop mode.
     * @param details
     *     If not NULL, this string will be set to a more detailed (technical)
     *     description of the model. This string may contain newlines.
     * @param params
     *     If not NULL, this variable will be set to a pointer to an array
     *     of lfParameter structures, every structure describes a model
     *     parameter. The list is NULL-terminated.
     * @return
     *     A short name of the distortion model or NULL if model is unknown.
     */
    static const char *GetCropDesc (
        lfCropMode mode, const char **details, const lfParameter ***params);

    /**
     * @brief Get the human-readable lens type name and a short description of this
     * lens type.
     * @param type
     *     Lens type.
     * @param details
     *     If not NULL, this string will be set to a more detailed (technical)
     *     description of the lens type. This string may contain newlines.
     * @return
     *     A short name of the lens type or NULL if model is unknown.
     */
    static const char *GetLensTypeDesc (lfLensType type, const char **details);

    /**
     * @brief Interpolate lens geometry distortion data for given focal length.
     * @param focal
     *     The focal length in mm at which we need geometry distortion parameters.
     * @param res
     *     The resulting interpolated model.
     */
    bool InterpolateDistortion (float focal, lfLensCalibDistortion &res) const;

    /**
     * @brief Interpolate lens TCA calibration data for given focal length.
     * @param focal
     *     The focal length in mm at which we need TCA parameters.
     * @param res
     *     The resulting interpolated model.
     */
    bool InterpolateTCA (float focal, lfLensCalibTCA &res) const;

    /**
     * @brief Interpolate lens vignetting model parameters for given focal length,
     * aperture, and focus distance.
     * @param focal
     *     The focal length in mm for which we need vignetting parameters.
     * @param aperture
     *     The aperture (f-number) for which we need vignetting parameters.
     * @param distance
     *     The focus distance in meters (distance > 0) for which we need
     *     vignetting parameters.
     * @param res
     *     The resulting interpolated model.
     */
    bool InterpolateVignetting (
        float focal, float aperture, float distance, lfLensCalibVignetting &res) const;

    /**
     * @brief Interpolate lens crop data for given focal length.
     * @param focal
     *     The focal length in mm at which we need image parameters.
     * @param res
     *     The resulting interpolated information data.
     */
    bool InterpolateCrop (float focal, lfLensCalibCrop &res) const;

    /**
     * @brief Interpolate lens fov data for given focal length.
     *
     * The Field of View (FOV) database entry is deprecated since Lensfun 
     * version 0.3 and will be removed in future releases.
     *
     * @param focal
     *     The focal length in mm at which we need image parameters.
     * @param res
     *     The resulting interpolated information data.
     */
    bool InterpolateFov (float focal, lfLensCalibFov &res) const;
#endif
};

C_TYPEDEF (struct, lfLens)

/**
 * @brief Create a new lens object.
 * @return
 *     A new empty lens object.
 * @sa
 *     lfLens::lfLens
 */
LF_EXPORT lfLens *lf_lens_new ();

/**
 * @brief Destroy a lfLens object.
 *
 * This is equivalent to C++ "delete lens".
 * @param lens
 *     The lens object to destroy.
 * @sa
 *     lfLens::~lfLens
 */
LF_EXPORT void lf_lens_destroy (lfLens *lens);

/**
 * @brief Copy the data from one lfLens structure into another.
 * @param dest
 *     The destination object
 * @param source
 *     The source object
 * @sa
 *     lfLens::operator = (const lfCamera &)
 */
LF_EXPORT void lf_lens_copy (lfLens *dest, const lfLens *source);

/** @sa lfLens::Check */
LF_EXPORT cbool lf_lens_check (lfLens *lens);

/** @sa lfLens::GuessParameters */
LF_EXPORT void lf_lens_guess_parameters (lfLens *lens);

/** @sa lfLens::GetDistortionModelDesc */
LF_EXPORT const char *lf_get_distortion_model_desc (
    enum lfDistortionModel model, const char **details, const lfParameter ***params);

/** @sa lfLens::GetTCAModelDesc */
LF_EXPORT const char *lf_get_tca_model_desc (
    enum lfTCAModel model, const char **details, const lfParameter ***params);

/** @sa lfLens::GetVignettingModelDesc */
LF_EXPORT const char *lf_get_vignetting_model_desc (
    enum lfVignettingModel model, const char **details, const lfParameter ***params);

/** @sa lfLens::GetCropDesc */
LF_EXPORT const char *lf_get_crop_desc (
    enum lfCropMode mode, const char **details, const lfParameter ***params);

/** @sa lfLens::GetLensTypeDesc */
LF_EXPORT const char *lf_get_lens_type_desc (
    enum lfLensType type, const char **details);

/** @sa lfLens::InterpolateDistortion */
LF_EXPORT cbool lf_lens_interpolate_distortion (const lfLens *lens, float focal,
    lfLensCalibDistortion *res);

/** @sa lfLens::InterpolateTCA */
LF_EXPORT cbool lf_lens_interpolate_tca (const lfLens *lens, float focal, lfLensCalibTCA *res);

/** @sa lfLens::InterpolateVignetting */
LF_EXPORT cbool lf_lens_interpolate_vignetting (const lfLens *lens, float focal, float aperture,
    float distance, lfLensCalibVignetting *res);

/** @sa lfLens::InterpolateCrop */
LF_EXPORT cbool lf_lens_interpolate_crop (const lfLens *lens, float focal,
    lfLensCalibCrop *res);

/** @sa lfLens::InterpolateFov */
LF_EXPORT cbool lf_lens_interpolate_fov (const lfLens *lens, float focal,
    lfLensCalibFov *res);

/** @sa lfLens::AddCalibDistortion */
LF_EXPORT void lf_lens_add_calib_distortion (lfLens *lens, const lfLensCalibDistortion *dc);

/** @sa lfLens::RemoveCalibDistortion */
LF_EXPORT cbool lf_lens_remove_calib_distortion (lfLens *lens, int idx);

/** @sa lfLens::AddCalibTCA */
LF_EXPORT void lf_lens_add_calib_tca (lfLens *lens, const lfLensCalibTCA *tcac);

/** @sa lfLens::RemoveCalibTCA */
LF_EXPORT cbool lf_lens_remove_calib_tca (lfLens *lens, int idx);

/** @sa lfLens::AddCalibVignetting */
LF_EXPORT void lf_lens_add_calib_vignetting (lfLens *lens, const lfLensCalibVignetting *vc);

/** @sa lfLens::RemoveCalibVignetting */
LF_EXPORT cbool lf_lens_remove_calib_vignetting (lfLens *lens, int idx);

/** @sa lfLens::AddCalibCrop */
LF_EXPORT void lf_lens_add_calib_crop (lfLens *lens, const lfLensCalibCrop *cc);

/** @sa lfLens::RemoveCalibCrop */
LF_EXPORT cbool lf_lens_remove_calib_crop (lfLens *lens, int idx);

/** @sa lfLens::AddCalibFov */
LF_EXPORT void lf_lens_add_calib_fov (lfLens *lens, const lfLensCalibFov *cf);

/** @sa lfLens::RemoveCalibFov */
LF_EXPORT cbool lf_lens_remove_calib_fov (lfLens *lens, int idx);

/** @} */

/**
 * @brief A lens database object.
 *
 * This object contains a list of mounts, cameras and lenses through
 * which you can search. The objects are loaded from XML files
 * (see @ref dbsearch).
 *
 * Objects loaded later override objects loaded earlier.
 * Thus, if user modifies a object you must save it to its home directory
 * (see lfDatabase::UserLocation), where it will override any definitions
 * from the system-wide database.
 *
 */
struct LF_EXPORT lfDatabase
{
#ifdef __cplusplus

    lfDatabase ();
    ~lfDatabase ();

    // /**
    //  * @brief Find a set of cameras that fit given criteria.
    //  *
    //  * The maker and model must be given (if possible) exactly as they are
    //  * spelled in database, except that the library will compare
    //  * case-insensitively and will compress spaces. This means that the
    //  * database must contain camera maker/lens *exactly* how it is given
    //  * in EXIF data, but you may add human-friendly translations of them
    //  * using the multi-language string feature (including a translation
    //  * to "en" to avoid displaying EXIF tags in user interface - they are
    //  * often upper-case which looks ugly).
    //  * @param maker
    //  *     Camera maker (either from EXIF tags or from some other source).
    //  *     The string is expected to be pure ASCII, since EXIF data does
    //  *     not allow 8-bit data to be used.
    //  * @param model
    //  *     Camera model (either from EXIF tags or from some other source).
    //  *     The string is expected to be pure ASCII, since EXIF data does
    //  *     not allow 8-bit data to be used.
    //  * @return
    //  *     A NULL-terminated list of cameras matching the search criteria
    //  *     or NULL if none. Release return value with lf_free() (only the list
    //  *     of pointers, not the camera objects!).
    //  */
    // const lfCamera **FindCameras (const char *maker, const char *model) const;

    // /**
    //  * @brief Searches all translations of camera maker and model.
    //  *
    //  * Thus, you may search for a user-entered camera even in a language
    //  * different from English.  This function is somewhat similar to
    //  * FindCameras(), but uses a different search algorithm.
    //  *
    //  * This is a lot slower than FindCameras().
    //  * @param maker
    //  *     Camera maker. This can be any UTF-8 string.
    //  * @param model
    //  *     Camera model. This can be any UTF-8 string.
    //  * @param sflags
    //  *     Additional flags influencing the search algorithm.
    //  *     This is a combination of LF_SEARCH_XXX flags.
    //  * @return
    //  *     A NULL-terminated list of cameras matching the search criteria
    //  *     or NULL if none. Release return value with lf_free() (only the list
    //  *     of pointers, not the camera objects!).
    //  */
    // const lfCamera **FindCamerasExt (const char *maker, const char *model,
    //                                  int sflags = 0) const;

    // /**
    //  * @brief Retrieve a full list of cameras.
    //  * @return
    //  *     An NULL-terminated list containing all cameras loaded until now.
    //  *     The list is valid only until the lens database is modified.
    //  *     The returned pointer does not have to be freed.
    //  */
    // const lfCamera *const *GetCameras () const;

    // /**
    //  * @brief Parse a human-friendly lens description (ex: "smc PENTAX-F 35-105mm F4-5.6"
    //  * or "SIGMA AF 28-300 F3.5-5.6 DL IF") and return a list of lfLens'es which
    //  * are matching this description.
    //  *
    //  * Multiple lenses may be returned if multiple lenses match (perhaps due to
    //  * non-unique lens description provided, e.g.  "Pentax SMC").
    //  *
    //  * The matching algorithm works as follows: first the user description
    //  * is tried to be interpreted according to several well-known lens naming
    //  * schemes, so additional data like focal and aperture ranges are extracted
    //  * if they are present. After that word matching is done; a lens matches
    //  * the description ONLY IF it contains all the words found in the description
    //  * (including buzzwords e.g. IF IZ AL LD DI USM SDM etc). Order of the words
    //  * does not matter. An additional check is done on the focal/aperture ranges,
    //  * they must exactly match if they are specified.
    //  * @param camera
    //  *     The camera (can be NULL if camera is unknown, or just certain
    //  *     fields in structure can be NULL). The algorithm will look for
    //  *     a lens with crop factor not larger than of given camera, since
    //  *     the mathematical models of the lens can be incorrect for sensor
    //  *     sizes larger than the one used for calibration. Also camera
    //  *     mount is taken into account, the lenses with incompatible
    //  *     mounts will be filtered out.
    //  * @param maker
    //  *     Lens maker or NULL if not known.
    //  * @param model
    //  *     A human description of the lens model(-s).
    //  * @param sflags
    //  *     Additional flags influencing the search algorithm.
    //  *     This is a combination of LF_SEARCH_XXX flags.
    //  * @return
    //  *     A list of lenses parsed from user description or NULL.
    //  *     Release memory with lf_free(). The list is ordered in the
    //  *     most-likely to least-likely order, e.g. the first returned
    //  *     value is the most likely match.
    //  */
    // const lfLens **FindLenses (const lfCamera *camera, const char *maker,
    //                            const char *model, int sflags = 0) const;

    // /**
    //  * @brief Find a set of lenses that fit certain criteria.
    //  * @param lens
    //  *     The approximative lense. Uncertain fields may be NULL.
    //  *     The "CropFactor" field defines the minimal value for crop factor;
    //  *     no lenses with crop factor larger than that will be returned.
    //  *     The Mounts field will be scanned for allowed mounts, if NULL
    //  *     any mounts are considered compatible.
    //  * @param sflags
    //  *     Additional flags influencing the search algorithm.
    //  *     This is a combination of LF_SEARCH_XXX flags.
    //  * @return
    //  *     A NULL-terminated list of lenses matching the search criteria
    //  *     or NULL if none. Release memory with lf_free(). The list is ordered
    //  *     in the most-likely to least-likely order, e.g. the first returned
    //  *     value is the most likely match.
    //  */
    // const lfLens **FindLenses (const lfLens *lens, int sflags = 0) const;

    // /**
    //  * @brief Retrieve a full list of lenses.
    //  * @return
    //  *     An NULL-terminated list containing all lenses loaded until now.
    //  *     The list is valid only until the lens database is modified.
    //  *     The returned pointer does not have to be freed.
    //  */
    // const lfLens *const *GetLenses () const;

    // /**
    //  * @brief Return the lfMount structure given the (basic) mount name.
    //  * @param mount
    //  *     The basic mount name.
    //  * @return
    //  *     A pointer to lfMount structure or NULL.
    //  */
    // const lfMount *FindMount (const char *mount) const;

    // /**
    //  * @brief Get the name of a mount in current locale.
    //  * @param mount
    //  *     The basic mount name.
    //  * @return
    //  *     The name of the mount in current locale (UTF-8 string).
    //  */
    // const char *MountName (const char *mount) const;

    // /**
    //  * @brief Retrieve a full list of mounts.
    //  * @return
    //  *     An array containing all mounts loaded until now.
    //  *     The list is valid only until the mount database is modified.
    //  *     The returned pointer does not have to be freed.
    //  */
    // const lfMount *const *GetMounts () const;

    // /**
    //  * @brief Add a mount to the database.
    //  * @param mount
    //  *     the mount to add
    //  */
    // void AddMount (lfMount *mount);

    // /**
    //  * @brief Add a camera to the database.
    //  * @param camera
    //  *     the camera to add
    //  */
    // void AddCamera (lfCamera *camera);

    // /**
    //  * @brief Add a lens to the database.
    //  * @param lens
    //  *     the lens to add
    //  */
    // void AddLens (lfLens *lens);

private:
#endif
    void *Mounts;
    void *Cameras;
    void *Lenses;
};

C_TYPEDEF (struct, lfDatabase)

// /**
//  * @brief Create a new empty database object.
//  *
//  * Usually the application will want to do this at startup,
//  * after which it would be a good idea to call lf_db_load().
//  * @return
//  *     A new empty database object.
//  * @sa
//  *     lfDatabase::lfDatabase
//  */
// LF_EXPORT lfDatabase *lf_db_new (void);

// /**
//  * @brief Destroy the database object.
//  *
//  * This is the only way to correctly destroy the database object.
//  * @param db
//  *     The database to destroy.
//  * @sa
//  *     lfDatabase::~lfDatabase
//  */
// LF_EXPORT void lf_db_destroy (lfDatabase *db);

// /** @sa lfDatabase::FindCameras */
// LF_EXPORT const lfCamera **lf_db_find_cameras (
//     const lfDatabase *db, const char *maker, const char *model);

// /** @sa lfDatabase::FindCamerasExt */
// LF_EXPORT const lfCamera **lf_db_find_cameras_ext (
//     const lfDatabase *db, const char *maker, const char *model, int sflags);

// /** @sa lfDatabase::GetCameras */
// LF_EXPORT const lfCamera *const *lf_db_get_cameras (const lfDatabase *db);

// /** @sa lfDatabase::FindLenses(const lfCamera *, const char *, const char *) */
// LF_EXPORT const lfLens **lf_db_find_lenses_hd (
//     const lfDatabase *db, const lfCamera *camera, const char *maker,
//     const char *lens, int sflags);

// /** @sa lfDatabase::FindLenses(const lfCamera *, const lfLens *) */
// LF_EXPORT const lfLens **lf_db_find_lenses (
//     const lfDatabase *db, const lfLens *lens, int sflags);

// /** @sa lfDatabase::GetLenses */
// LF_EXPORT const lfLens *const *lf_db_get_lenses (const lfDatabase *db);

// /** @sa lfDatabase::FindMount */
// LF_EXPORT const lfMount *lf_db_find_mount (const lfDatabase *db, const char *mount);

// /** @sa lfDatabase::MountName */
// LF_EXPORT const char *lf_db_mount_name (const lfDatabase *db, const char *mount);

// /** @sa lfDatabase::GetMounts */
// LF_EXPORT const lfMount *const *lf_db_get_mounts (const lfDatabase *db);

/** @} */

/*----------------------------------------------------------------------------*/

/**
 * @defgroup Correction Image correction
 * @brief This group of functions will allow you to apply a image transform
 * that will correct some lens defects.
 * @{
 */

/** @brief A list of bitmask flags used for ordering various image corrections */
enum
{
    /** Correct (or apply) lens transversal chromatic aberrations */
    LF_MODIFY_TCA        = 0x00000001,
    /** Correct (or apply) lens vignetting */
    LF_MODIFY_VIGNETTING = 0x00000002,
    /* Value 0x00000004 is deprecated. */
    /** Correct (or apply) lens distortion */
    LF_MODIFY_DISTORTION = 0x00000008,
    /** Convert image geometry */
    LF_MODIFY_GEOMETRY   = 0x00000010,
    /** Additional resize of image */
    LF_MODIFY_SCALE      = 0x00000020,
    /** Apply all possible corrections */
    LF_MODIFY_ALL        = ~0
};

/** @brief A list of pixel formats supported by internal colour callbacks */
enum lfPixelFormat
{
    /** Unsigned 8-bit R,G,B */
    LF_PF_U8,
    /** Unsigned 16-bit R,G,B */
    LF_PF_U16,
    /** Unsigned 32-bit R,G,B */
    LF_PF_U32,
    /** 32-bit floating-point R,G,B */
    LF_PF_F32,
    /** 64-bit floating-point R,G,B */
    LF_PF_F64
};

C_TYPEDEF (enum, lfPixelFormat)

/** @brief These constants define the role of every pixel component, four bits
 * each.  "pixel" refers here to a set of values which share the same (x, y)
 * coordinates. */
enum lfComponentRole
{
    /**
     * This marks the end of the role list. It doesn't have to be specified
     * explicitly, since LF_CR_X macros always pad the value with zeros
     */
    LF_CR_END = 0,
    /**
     * This value tells that what follows applies to next pixel.
     * This can be used to define Bayer images, e.g. use
     * LF_CR_3(LF_CR_RED, LF_CR_NEXT, LF_CR_GREEN) for even rows and
     * LF_CR_3(LF_CR_GREEN, LF_CR_NEXT, LF_CR_BLUE) for odd rows.
     */
    LF_CR_NEXT,
    /** This component has an unknown/doesn't matter role */
    LF_CR_UNKNOWN,
    /** This is the pixel intensity (grayscale) */
    LF_CR_INTENSITY,
    /** This is the Red pixel component */
    LF_CR_RED,
    /** This is the Green pixel component */
    LF_CR_GREEN,
    /** This is the Blue pixel component */
    LF_CR_BLUE
};

C_TYPEDEF (enum, lfComponentRole)

/** @brief This macro defines a pixel format consisting of one component */
#define LF_CR_1(a)              (LF_CR_ ## a)
/** @brief This macro defines a pixel format consisting of two components */
#define LF_CR_2(a,b)            ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4))
/** @brief This macro defines a pixel format consisting of three components */
#define LF_CR_3(a,b,c)          ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8))
/** @brief This macro defines a pixel format consisting of four components */
#define LF_CR_4(a,b,c,d)        ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8) | ((LF_CR_ ## d) << 12))
/** @brief This macro defines a pixel format consisting of five components */
#define LF_CR_5(a,b,c,d,e)      ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8) | ((LF_CR_ ## d) << 12) | \
                                 ((LF_CR_ ## e) << 16))
/** @brief This macro defines a pixel format consisting of six components */
#define LF_CR_6(a,b,c,d,e,f)    ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8) | ((LF_CR_ ## d) << 12) | \
                                 ((LF_CR_ ## e) << 16) | ((LF_CR_ ## f) << 20))
/** @brief This macro defines a pixel format consisting of seven components */
#define LF_CR_7(a,b,c,d,e,f,g)   ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8) | ((LF_CR_ ## d) << 12) | \
                                 ((LF_CR_ ## e) << 16) | ((LF_CR_ ## f) << 20) | \
                                 ((LF_CR_ ## g) << 24))
/** @brief This macro defines a pixel format consisting of eight components */
#define LF_CR_8(a,b,c,d,e,f,g,h) ((LF_CR_ ## a) | ((LF_CR_ ## b) << 4) | \
                                 ((LF_CR_ ## c) << 8) | ((LF_CR_ ## d) << 12) | \
                                 ((LF_CR_ ## e) << 16) | ((LF_CR_ ## f) << 20) | \
                                 ((LF_CR_ ## g) << 24) | ((LF_CR_ ## h) << 28))

/**
 * @brief A callback function which modifies the separate coordinates for all color
 * components for every pixel in a strip.
 *
 * This kind of callbacks are used in the second stage of image modification.
 * @param data
 *     A opaque pointer to some data.
 * @param iocoord
 *     A pointer to an array of count*3 pixel coordinates (X,Y).
 *     The first coordinate pair is for the R component, second for G
 *     and third for B of same pixel. There are count*2*3 floats total
 *     in this array.
 * @param count
 *     Number of coordinate groups to handle.
 */
typedef void (*lfSubpixelCoordFunc) (void *data, float *iocoord, int count);

/**
 * @brief A callback function which modifies the colors of a strip of pixels
 *
 * This kind of callbacks are used in the first stage of image modification.
 * @param data
 *     A opaque pointer to some data.
 * @param x
 *     The X coordinate of the beginning of the strip. For next pixels
 *     the X coordinate increments sequentialy.
 * @param y
 *     The Y coordinate of the pixel strip. This is a constant across
 *     all pixels of the strip.
 * @param pixels
 *     A pointer to pixel data. It is the responsability of the function
 *     inserting the callback into the chain to provide a callback operating
 *     with the correct pixel format.
 * @param comp_role
 *     The role of every pixel component. This is a bitfield, made by one
 *     of the LF_CR_X macros which defines the roles of every pixel field.
 *     For example, LF_CR_4(RED,GREEN,BLUE,UNKNOWN) will define a RGBA
 *     (or RGBX) pixel format, and the UNKNOWN field will not be modified.
 * @param count
 *     Number of pixels to process.
 */
typedef void (*lfModifyColorFunc) (void *data, float x, float y,
                                   void *pixels, int comp_role, int count);

/**
 * @brief A callback function which modifies the coordinates of a strip of pixels.
 *
 * This kind of callbacks are used in third stage of image modification.
 * @param data
 *     A opaque pointer to some data.
 * @param iocoord
 *     A pointer to an array @a count pixel coordinates (X,Y).
 *     The function must replace the coordinates with their new values.
 * @param count
 *     Number of coordinate groups to handle.
 */
typedef void (*lfModifyCoordFunc) (void *data, float *iocoord, int count);

// @cond
    
/// Common ancestor for lfCoordCallbackData and lfColorCallbackData
struct lfCallbackData
{
    int priority;
    void *data;
    size_t data_size;
};

// A test point in the autoscale algorithm
typedef struct { float angle, dist; } lfPoint;

// @endcond

/**
 * @brief A modifier object contains optimized data required to rectify a
 * image.
 *
 * You can either create an empty modifier object and then enable the
 * required modification functions individually, or you can take
 * a lens object, which contains a set of correction models, and
 * create a modifier object from it.
 *
 * Normally, you will create an instance of lfModifier and initialise
 * immediately it with lfModifier::Initialize, passing a valid lens object in
 * both cases.  Users of the plain C interface will use lf_modifier_new() and
 * lf_modifier_initialize() instead.
 *
 * Every image modification has a corresponding inverse function,
 * e.g. the library allows both to correct lens distortions and to
 * simulate lens characteristics.
 *
 * The normal order of applying a lens correction on a image is:
 * <ol>
 * <li>Fix lens vignetting
 * <li>Fix chromatic aberrations (TCA)
 * <li>Fix lens distortion
 * <li>Fix lens geometry
 * <li>Fix perspective
 * <li>Scale the image
 * </ol>
 *
 * This is the theoretical order.  But in reality, when using this library, the
 * order is reversed (see \ref corrections for an explanation):
 *
 * <ol>
 * <li>Color<ol>
 * <li>Fix lens vignetting
 * </ol>
 * <li>Coordinates<ol>
 * <li>Scale the image
 * <li>Fix perspective
 * <li>Fix lens geometry
 * <li>Fix lens distortion
 * </ol>
 * <li>Subpixel coordinates<ol>
 * <li>Fix chromatic aberrations (TCA)
 * </ol>
 * </ol>
 *
 * This process is divided into three stages.
 *
 * In the first stage, the colors of the image pixels are fixed
 * (vignetting). You pass a pointer to your pixel data, and it will be modified
 * in place.
 *
 * Then, the distortions introduced by the lens are removed (distortion and
 * geometry), perspective is corrected if desired, and an additional scaling
 * factor is applied if required.  This operation requires building new image
 * in a new allocated buffer: you cannot modify the image in place, or bad
 * things will happen.  Note that Lensfun does not provide pixel interpolation
 * routines.  You have to implement the lookup in the original image yourself.
 *
 * And finally, in the subpixel distortion stage, application corrects
 * transversal chromatic aberrations. For every target pixel coordinate the
 * modifier will return you three new coordinates: the first will tell you the
 * coordinates of the red component, second of the green, and third of the blue
 * component. This again requires copying the image, but you can use the same
 * buffers as in stage two, just in reverse order.
 *
 * Of course, you can skip some stages of the process, e.g. if you, for
 * example, don't want to change a fisheye image to a rectilinear you
 * can omit that step.
 *
 * Obviously, when simulating lens distortions, the modification stages
 * must be applied in reverse order. While the library takes care to reverse
 * the steps which are grouped into a single stage, the application must
 * apply the stages themselves in reverse order.
 *
 * HOWEVER. Doing it in three stages is not memory efficient, and is prone to
 * error accumulation because you have to interpolate pixels twice - once
 * during stage 2 and once during stage 3. To avoid this, it is sensful to do
 * stages 2 & 3 in one step.  In this case the output R,G,B coordinates from
 * stage 2, which treats the colour channels equally, are fed directly into
 * stage 3, which will correct the R,G,B coordinates further.
 * ApplySubpixelGeometryDistortion() does this in a convenient fashion.
 */
#ifdef __cplusplus
}
#endif

struct LF_EXPORT lfModifier
{
#ifdef __cplusplus
    /**
     * @brief Create an empty image modifier object.
     *
     * Before using the returned object you must add the required
     * modifier callbacks (see methods AddXXXCallback below).
     *
     * You must provide the original image width/height even if you
     * plan to correct just a part of the image.
     *
     * @param lens
     *     For all modifications, the crop factor, aspect ratio, and
     *     center shift of this lens will be used.
     * @param crop
     *     The crop factor for current camera. The distortion models will
     *     take this into account if lens models were measured on a camera
     *     with a different crop factor.
     * @param width
     *     The width of the image you want to correct.
     * @param height
     *     The height of the image you want to correct.
     */
    lfModifier (const lfLens *lens, float crop, int width, int height);
    ~lfModifier ();

    /**
     * @brief Initialize the process of correcting aberrations in a image.
     *
     * The modifier object will be set up to rectify all aberrations
     * found in the lens description. Make sure the focal length,
     * aperture, and focus distance are correct in order to ensure
     * proper rectification.
     *
     * Aperture and focus distance are only used for vignetting
     * correction. Because the dependence of vignetting on focus
     * distance is very small, and it is usually not available in EXIF
     * data, you may give an approximative value here.  If you really
     * do not know, as a last resort, use "1000" as a default value.
     * @param lens
     *     The lens which aberrations you want to correct in a image.
     *     It should be the same lens object as the one passed to
     *     the lfModifier constructor.
     * @param format
     *     Pixel format of your image (bits per pixel component)
     * @param focal
     *     The focal length in mm at which the image was taken.
     * @param aperture
     *     The aperture (f-number) at which the image was taken.
     * @param distance
     *     The approximative focus distance in meters (distance > 0).
     * @param scale
     *     An additional scale factor to be applied onto the image
     *     (1.0 - no scaling; 0.0 - automatic scaling).
     * @param targeom
     *     Target geometry. If LF_MODIFY_GEOMETRY is set in @a flags and
     *     @a targeom is different from lens->Type, a geometry conversion
     *     will be applied on the image.
     * @param flags
     *     A set of flags (se LF_MODIFY_XXX) telling which distortions
     *     you want corrected. A value of LF_MODIFY_ALL orders correction
     *     of everything possible (will enable all correction models
     *     present in lens description).
     * @param reverse
     *     If this parameter is true, a reverse transform will be prepared.
     *     That is, you take a undistorted image at input and convert it so
     *     that it will look as if it would be a shot made with @a lens.
     * @return
     *     A set of LF_MODIFY_XXX flags in effect. This is the @a flags argument
     *     with dropped bits for operations which are actually no-ops.
     */
    int Initialize (
        const lfLens *lens, lfPixelFormat format, float focal, float aperture,
        float distance, float scale, lfLensType targeom, int flags, bool reverse);

    /**
     * @brief Enable the perspective correction.
     *
     * Depending on the number of control points given, there are three
     * possible modes:
     *
     * 4 control points: c0 and c1 define one vertical lines, c2 and c3 the
     * other.  The focal length defined in the creation of the modifier is used
     * to get the proper aspect ratio.
     *
     * 6 control points: c0 to c3 like above.  c4 and c5 define a horizontal
     * line.  The focal length is used to get the proper aspect ratio.
     *
     * 8 control points: c0 to c5 like above.  c6 and c7 define a second
     * horizontal line.  The focal length is not used.
     *
     * 5 control points: They all must lie on an ellipse that is actually a
     * rotated circle.  If they lie in a clockwise ordering, the vertex is
     * assumed to be above the ellipse centre in the image.  If they lie in a
     * counter-clockwise ordering, the vertex is assumed to be below the
     * ellipse.  The focal length is needed to find the proper vertex.
     *
     * 7 control points: c0 to c4 like above.  c5 and c6 define a horizontal
     * line which is used to rotate the final image nicely.
     *
     * If the lines constructed from the first four control points for the 4, 6
     * and 8 points case (or the last two for the 7 points case) are more
     * horizontal than vertical, in all of the above, "horizontal" and
     * "vertical" need to be swapped.
     *
     * All control points must be given as pixel coordinates in the original
     * image.  For best precision, anti-distortion should have been applied
     * before taking the coordinates.  Moreover, fisheye images must have been
     * transformed into rectilinear of the same focal length.  In contrast,
     * cropping, rotating, shifting, or scaling must be switched off.
     *
     * @param x
     *     The x coordinates of the control points.
     * @param y
     *     The y coordinates of the control points.
     * @param count
     *     The number of control points.
     * @param d
     *     This parameter is supposed to be offered to the user as a slider.
     *     It can take values from -1 to +1.  0 denotes the perfect correction.
     *     -1 is the unchanged image.  +1 is an increase of the tilting angle
     *     by 25%.
     * @return
     *     True if the perspective correction was enabled.
     */
    bool EnablePerspectiveCorrection (float *x, float *y, int count, float d);

    /**
     * @brief Add a user-defined callback to the coordinate correction chain.
     * @param callback
     *     The callback to be called for every strip of pixels.
     * @param priority
     *     Callback priority (0-999). Callbacks are always called in
     *     increasing priority order.
     * @param data
     *     A pointer to additional user data. A copy of this data
     *     is made internally, so client application may do whatever
     *     it needs with this data after this call.
     * @param data_size
     *     User data size in bytes. If data size is zero, the data is not
     *     copied and instead a verbatim copy of the 'data' parameter
     *     is passed to the callback.
     */
    void AddCoordCallback (lfModifyCoordFunc callback, int priority,
                           void *data, size_t data_size);

    /**
     * @brief Add a user-defined callback to the subpixel coordinate
     * rectification chain.
     * @param callback
     *     The callback to be called for every strip of pixels.
     * @param priority
     *     Callback priority (0-999). Callbacks are always called in
     *     increasing priority order.
     * @param data
     *     A pointer to additional user data. A copy of this data
     *     is made internally, so client application may do whatever
     *     it needs with this data after this call.
     * @param data_size
     *     User data size in bytes. If data size is zero, the data is not
     *     copied and instead a verbatim copy of the 'data' parameter
     *     is passed to the callback.
     */
    void AddSubpixelCallback (lfSubpixelCoordFunc callback, int priority,
                              void *data, size_t data_size);

    /**
     * @brief Add a user-defined callback to the color modification chain.
     * @param callback
     *     The callback to be called for every strip of pixels.
     * @param priority
     *     Callback priority (0-999). Callbacks are always called in
     *     increasing priority order.
     * @param data
     *     A pointer to additional user data. A copy of this data
     *     is made internally, so client application may do whatever
     *     it needs with this data after this call.
     * @param data_size
     *     User data size in bytes. If data size is zero, the data is not
     *     copied and instead a verbatim copy of the 'data' parameter
     *     is passed to the callback.
     */
    void AddColorCallback (lfModifyColorFunc callback, int priority,
                           void *data, size_t data_size);

    /**
     * @brief Add the stock TCA correcting callback into the chain.
     *
     * The TCA correction callback always has a fixed priority of 500.
     * The behaviour is undefined if you'll add more than one TCA
     * correction callback to a modifier.
     * @param model
     *     Lens TCA model data.
     * @param reverse
     *     If true, the reverse model will be applied, e.g. simulate
     *     a lens' TCA on a clean image.
     * @return
     *     True if TCA model is valid and the callback was added to chain.
     */
    bool AddSubpixelCallbackTCA (lfLensCalibTCA &model, bool reverse = false);

    /**
     * @brief Add the stock lens vignetting correcting callback into the chain.
     * The vignetting correction callback always has a fixed priority of
     * 250 when rectifying a image or 750 when doing reverse transform.
     * @param model
     *     Lens vignetting model data.
     * @param format
     *     Pixel format of your image (bits per pixel component)
     * @param reverse
     *     If true, the reverse model will be applied, e.g. simulate
     *     a lens' vignetting on a clean image.
     * @return
     *     True if vignetting model is valid and the callback was added to chain.
     */
    bool AddColorCallbackVignetting (lfLensCalibVignetting &model, lfPixelFormat format,
                                     bool reverse = false);

    /**
     * @brief Add the stock lens distortion correcting callback into the chain.
     *
     * The distortion correction callback always has a fixed priority of 750
     * when rectifying a image and 250 on reverse transform.
     * @param model
     *     Lens distortion model data.
     * @param reverse
     *     If true, the reverse model will be applied, e.g. simulate
     *     a lens' distortion on a clean image.
     * @return
     *     True if distortion model is valid and the callback was added to chain.
     */
    bool AddCoordCallbackDistortion (lfLensCalibDistortion &model, bool reverse = false);

    /**
     * @brief Add the stock lens geometry rectification callback into the
     * chain.
     *
     * The geometry correction callback always has a fixed priority of 500.
     * @param from
     *     The lens model for source image.
     * @param to
     *     The lens model for target image.
     * @param focal
     *     Lens focal length in mm.  Deprecated.
     * @return
     *     True if a library has a callback for given from->to conversion.
     */
    bool AddCoordCallbackGeometry (lfLensType from, lfLensType to, float focal = 0);

    /**
     * @brief Add the stock image scaling callback into the chain.
     *
     * The scaling callback always has a fixed priority of 100.
     * Note that scaling should be always the first operation to perform
     * no matter if we're doing a forward or reverse transform.
     * @param scale
     *     Image scale. If equal to 0.0, the image is automatically scaled
     *     so that there won't be any unfilled gaps in the resulting image.
     *     Note that all coordinate distortion callbacks must be already
     *     added to the stack for this to work correctly!
     * @param reverse
     *     If true, the reverse model will be applied.
     * @return
     *     True if the callback was added to chain.
     */
    bool AddCoordCallbackScale (float scale, bool reverse = false);

    /**
     * @brief Compute the automatic scale factor for the image.
     *
     * This expects that all coordinate distortion callbacks are already
     * set up and working. The function will try at its best to find a
     * scale value that will ensure that no pixels with coordinates out
     * of range will get into the resulting image. But since this is
     * an approximative method, the returned scale sometimes is a little
     * less than the optimal scale (e.g. you can still get some black
     * corners with some high-distortion cases).
     * @param reverse
     *     If true, the reverse scaling factor is computed.
     */
    float GetAutoScale (bool reverse);

    /**
     * @brief Image correction step 1: fix image colors.
     *
     * This currently is only vignetting transform.
     * @param pixels
     *     This points to image pixels. The actual pixel format depends on both
     *     pixel_format and comp_role arguments.
     *     The results are stored in place in the same format.
     *     Warning: this array should be aligned at least on a 16-byte boundary.
     * @param x
     *     The X coordinate of the corner of the block.
     * @param y
     *     The Y coordinate of the corner of the block.
     * @param width
     *     The width of the image block in pixels.
     * @param height
     *     The height of the image block in pixels.
     * @param comp_role
     *     The role of every pixel component. This is a bitfield, made by one
     *     of the LF_CR_X macros which defines the roles of every pixel field.
     *     For example, LF_CR_4(RED,GREEN,BLUE,UNKNOWN) will define a RGBA
     *     (or RGBX) pixel format, and the UNKNOWN field will not be modified.
     * @param row_stride
     *     The size of a image row in bytes. This can be actually different
     *     from width * pixel_width as some image formats use funny things
     *     like alignments, filler bytes etc.
     * @return
     *     true if return buffer has been altered, false if nothing to do
     */
    bool ApplyColorModification (void *pixels, float x, float y, int width, int height,
                                 int comp_role, int row_stride) const;

    /**
     * @brief Image correction step 2: apply the transforms on a block of pixel
     * coordinates.
     *
     * The undistorted coordinates are computed for every pixel in a
     * rectangular block: \f$(x_u, y_u), (x_u+1, y_u), \ldots, (x_u +
     * \mathrm{width} - 1, y_u), (x_u, y_u + 1), \ldots, (x_u + \mathrm{width}
     * - 1, y_u + \mathrm{height} - 1)\f$.
     *
     * The corrected coordinates are put into the output buffer
     * sequentially, X and Y values.
     *
     * This routine has been designed to be safe to use in parallel from
     * several threads.
     * @param xu
     *     The undistorted X coordinate of the start of the block of pixels.
     * @param yu
     *     The undistorted Y coordinate of the start of the block of pixels.
     * @param width
     *     The width of the block in pixels.
     * @param height
     *     The height of the block in pixels.
     * @param res
     *     A pointer to an output array which receives the respective X and Y
     *     distorted coordinates for every pixel of the block. The size of
     *     this array must be at least width*height*2 elements.
     *     Warning: this array should be aligned at least on a 16-byte boundary.
     * @return
     *     true if return buffer has been filled, false if nothing to do
     */
    bool ApplyGeometryDistortion (float xu, float yu, int width, int height,
                                  float *res) const;

    /**
     * @brief Image correction step 3: apply subpixel distortions.
     *
     * The undistorted R,G,B coordinates are computed for every pixel in a
     * square block: \f$(x_u, y_u), (x_u+1, y_u), \ldots, (x_u +
     * \mathrm{width} - 1, y_u), (x_u, y_u + 1), \ldots, (x_u + \mathrm{width}
     * - 1, y_u + \mathrm{height} - 1)\f$.
     *
     * Returns the corrected coordinates separately for R/G/B channels
     * The resulting coordinates are put into the output buffer
     * sequentially, X and Y values.
     *
     * This routine has been designed to be safe to use in parallel from
     * several threads.
     * @param xu
     *     The undistorted X coordinate of the start of the block of pixels.
     * @param yu
     *     The undistorted Y coordinate of the start of the block of pixels.
     * @param width
     *     The width of the block in pixels.
     * @param height
     *     The height of the block in pixels.
     * @param res
     *     A pointer to an output array which receives the respective X and Y
     *     distorted coordinates of the red, green and blue channels for
     *     every pixel of the block. The size of this array must be
     *     at least width*height*2*3 elements.
     *     Warning: this array should be aligned at least on a 16-byte boundary.
     * @return
     *     true if return buffer has been filled, false if nothing to do
     */
    bool ApplySubpixelDistortion (float xu, float yu, int width, int height,
                                  float *res) const;

    /**
     * @brief Apply stage 2 & 3 in one step.
     *
     * See the main comment to the lfModifier class.  The undistorted (R, G, B)
     * coordinates are computed for every pixel in a square block: \f$(x_u,
     * y_u), (x_u+1, y_u), \ldots, (x_u + \mathrm{width} - 1, y_u), (x_u, y_u +
     * 1), \ldots, (x_u + \mathrm{width} - 1, y_u + \mathrm{height} - 1)\f$.
     *
     * Returns the corrected coordinates separately for R/G/B channels
     * The resulting coordinates are put into the output buffer
     * sequentially, X and Y values.
     *
     * This routine has been designed to be safe to use in parallel from
     * several threads.
     * @param xu
     *     The undistorted X coordinate of the start of the block of pixels.
     * @param yu
     *     The undistorted Y coordinate of the start of the block of pixels.
     * @param width
     *     The width of the block in pixels.
     * @param height
     *     The height of the block in pixels.
     * @param res
     *     A pointer to an output array which receives the respective X and Y
     *     distorted coordinates for every pixel of the block. The size of
     *     this array must be at least width*height*2*3 elements.
     *     Warning: this array should be aligned at least on a 16-byte boundary.
     * @return
     *     true if return buffer has been filled, false if nothing to do
     */
    bool ApplySubpixelGeometryDistortion (float xu, float yu, int width, int height,
                                          float *res) const;

private:
    /**
     * @brief Determine the real focal length.
     *
     * Returns the real focal length (in contrast to the nominal focal length
     * given in the "focal" attribute in the XML files).  This is the textbook
     * focal length, derived from the magnification in paraxial approximation.
     * It is needed for accurate geometry transformations, e.g. from fisheye to
     * rectilinear.  If there is neither real focal length nor FOV data
     * available, the nominal focal length is returned as a fallback.
     *
     * In practice, its effect is mostly negligible.  When converting to
     * rectilinear, it merely results in a magnification (which probably is
     * reverted by autoscaling).  When converting to a fisheye projection
     * besides stereographic, the degree of distortion is not detectable by the
     * human eye.  Moreover, most non-fisheyes have quite accurate nominal
     * focal lengths printed on the lens.  Thus, the only use case left is the
     * conversion from non-stereographic fisheye to stereographic.  This maps
     * perfect circled to perfect circles, so it is noticeable if the nominal
     * focal length is rather off.
     *
     * @param lens
     *     The lens for which the focal length should be returned.
     * @param focal
     *     The nominal focal length for which the real focal length should be
     *     returned.
     * @return
     *     the real focal length, or, if no real focal length data and no FOV
     *     data is included into the calibration, the given nominal focal
     *     length
     */
    float GetRealFocalLength (const lfLens *lens, float focal);

    void AddCallback (void *arr, lfCallbackData *d,
                      int priority, void *data, size_t data_size);

    /**
     * @brief Calculate distance between point and image edge.
     *
     * This is an internal function used for autoscaling.  It returns the
     * distance between the given point and the edge of the image.  The
     * coordinate system used is the normalized system.  Points inside the
     * image frame yield negative values, points outside positive values, and
     * on the frame zero.
     * @param coord
     *     The x, y coordinates of the points, as a 2-component array.
     * @return
     *     The distance of the point from the edge.
     */
    double AutoscaleResidualDistance (float *coord) const;
    /**
     * @brief Calculate distance of the corrected edge point from the centre.
     *
     * This is an internal function used for autoscaling.  It returns the
     * distance of the point on the edge of the corrected image which lies in
     * the direction of the given coordinates.  "Distance" means "distance from
     * origin".  This way, the necessary autoscaling value for this direction
     * can be calculated by the calling routine.
     * @param point
     *     The polar coordinates of the point for which the distance of the
     *     corrected counterpart should be calculated.
     * @return
     *     The distance of the corrected image edge from the origin.
     */
    float GetTransformedDistance (lfPoint point) const;

    static void ModifyCoord_UnTCA_Linear (void *data, float *iocoord, int count);
    static void ModifyCoord_TCA_Linear (void *data, float *iocoord, int count);
    static void ModifyCoord_UnTCA_Poly3 (void *data, float *iocoord, int count);
    static void ModifyCoord_TCA_Poly3 (void *data, float *iocoord, int count);
    static void ModifyCoord_TCA_ACM (void *data, float *iocoord, int count);

    static void ModifyCoord_UnDist_Poly3 (void *data, float *iocoord, int count);
    static void ModifyCoord_Dist_Poly3 (void *data, float *iocoord, int count);
#ifdef VECTORIZATION_SSE
    static void ModifyCoord_Dist_Poly3_SSE (void *data, float *iocoord, int count);
#endif
    static void ModifyCoord_UnDist_Poly5 (void *data, float *iocoord, int count);
    static void ModifyCoord_Dist_Poly5 (void *data, float *iocoord, int count);
    static void ModifyCoord_UnDist_PTLens (void *data, float *iocoord, int count);
    static void ModifyCoord_Dist_PTLens (void *data, float *iocoord, int count);
#ifdef VECTORIZATION_SSE
    static void ModifyCoord_UnDist_PTLens_SSE (void *data, float *iocoord, int count);
    static void ModifyCoord_Dist_PTLens_SSE (void *data, float *iocoord, int count);
#endif
    static void ModifyCoord_Dist_ACM (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_FishEye_Rect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Panoramic_Rect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Rect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Rect_FishEye (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Panoramic_FishEye (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_FishEye (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Rect_Panoramic (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_FishEye_Panoramic (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Panoramic (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Rect_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_FishEye_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Panoramic_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Orthographic_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Orthographic (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Stereographic_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Stereographic (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Equisolid_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Equisolid (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_Thoby_ERect (void *data, float *iocoord, int count);
    static void ModifyCoord_Geom_ERect_Thoby (void *data, float *iocoord, int count);
    static void ModifyCoord_Perspective_Correction (void *data, float *iocoord, int count);
#ifdef VECTORIZATION_SSE
    static void ModifyColor_DeVignetting_PA_SSE (
      void *data, float _x, float _y, lf_f32 *pixels, int comp_role, int count);
#endif
#ifdef VECTORIZATION_SSE2
    static void ModifyColor_DeVignetting_PA_SSE2 (
      void *data, float _x, float _y, lf_u16 *pixels, int comp_role, int count);
#endif

    template<typename T> static void ModifyColor_Vignetting_PA (
        void *data, float x, float y, T *rgb, int comp_role, int count);
    template<typename T> static void ModifyColor_DeVignetting_PA (
        void *data, float x, float y, T *rgb, int comp_role, int count);

    static void ModifyCoord_Scale (void *data, float *iocoord, int count);
#endif
    /// Coordinate grid width and height; these are the original image
    /// dimensions minus one.
    double Width, Height;
    /// The center of distortions in normalized coordinates
    double CenterX, CenterY;
    /// The coefficients for conversion to and from normalized coords
    double NormScale, NormUnScale;
    /// Factor to transform from normalized into absolute coords (mm).  Needed
    /// for calculating the real focal length from the FOV, and for calculating
    /// FocalLengthNormalized.
    double NormalizedInMillimeters;
    /// Used for conversion from distortion to vignetting coordinate system of
    /// the calibration sensor
    double AspectRatioCorrection;

    /// A list of subpixel coordinate modifier callbacks.
    void *SubpixelCallbacks;
    /// A list of pixel color modifier callbacks.
    void *ColorCallbacks;
    /// A list of pixel coordinate modifier callbacks.
    void *CoordCallbacks;

    /// Maximal x and y value in normalized coordinates for the original image
    double MaxX, MaxY;

    /// Nominal focal length (usually taken from EXIF data) in normalized
    /// units.  Currently, it is needed for geometry transformation, the Adobe
    /// camera models, and perspective correction.  On the long run, /all/
    /// image data will be stored in lfModifier, once the Enable... methods are
    /// written.
    double FocalLengthNormalized;
    /// Whether the transformations are applied reversely
    bool Reverse;
};

#ifdef __cplusplus
extern "C" {
#endif

C_TYPEDEF (struct, lfModifier)

/** @sa lfModifier::Create */
LF_EXPORT lfModifier *lf_modifier_new (
    const lfLens *lens, float crop, int width, int height);

/** @sa lfModifier::Destroy */
LF_EXPORT void lf_modifier_destroy (lfModifier *modifier);

/** @sa lfModifier::Initialize */
LF_EXPORT int lf_modifier_initialize (
    lfModifier *modifier, const lfLens *lens, lfPixelFormat format,
    float focal, float aperture, float distance, float scale,
    lfLensType targeom, int flags, cbool reverse);

/** @sa lfModifier::EnablePerspectiveCorrection */
LF_EXPORT cbool lf_modifier_enable_perspective_correction (
    lfModifier *modifier, float *x, float *y, int count, float d);

/** @sa lfModifier::AddCoordCallback */
LF_EXPORT void lf_modifier_add_coord_callback (
    lfModifier *modifier, lfModifyCoordFunc callback, int priority,
    void *data, size_t data_size);

/** @sa lfModifier::AddSubpixelCallback */
LF_EXPORT void lf_modifier_add_subpixel_callback (
    lfModifier *modifier, lfSubpixelCoordFunc callback, int priority,
    void *data, size_t data_size);

/** @sa lfModifier::AddColorCallback */
LF_EXPORT void lf_modifier_add_color_callback (
    lfModifier *modifier, lfModifyColorFunc callback, int priority,
    void *data, size_t data_size);

/** @sa lfModifier::AddSubpixelCallbackTCA */
LF_EXPORT cbool lf_modifier_add_subpixel_callback_TCA (
    lfModifier *modifier, lfLensCalibTCA *model, cbool reverse);

/** @sa lfModifier::AddColorCallbackVignetting */
LF_EXPORT cbool lf_modifier_add_color_callback_vignetting (
    lfModifier *modifier, lfLensCalibVignetting *model,
    lfPixelFormat format, cbool reverse);

/** @sa lfModifier::AddCoordCallbackDistortion */
LF_EXPORT cbool lf_modifier_add_coord_callback_distortion (
    lfModifier *modifier, lfLensCalibDistortion *model, cbool reverse);

/** @sa lfModifier::AddCoordCallbackGeometry */
LF_EXPORT cbool lf_modifier_add_coord_callback_geometry (
    lfModifier *modifier, lfLensType from, lfLensType to);

/** @sa lfModifier::AddCoordCallbackScale */
LF_EXPORT cbool lf_modifier_add_coord_callback_scale (
    lfModifier *modifier, float scale, cbool reverse);

/** @sa lfModifier::GetAutoScale */
LF_EXPORT float lf_modifier_get_auto_scale (
    lfModifier *modifier, cbool reverse);

/** @sa lfModifier::ApplySubpixelDistortion */
LF_EXPORT cbool lf_modifier_apply_subpixel_distortion (
    lfModifier *modifier, float xu, float yu, int width, int height, float *res);

/** @sa lfModifier::ApplyColorModification */
LF_EXPORT cbool lf_modifier_apply_color_modification (
    lfModifier *modifier, void *pixels, float x, float y, int width, int height,
    int comp_role, int row_stride);

/** @sa lfModifier::ApplyGeometryDistortion */
LF_EXPORT cbool lf_modifier_apply_geometry_distortion (
    lfModifier *modifier, float xu, float yu, int width, int height, float *res);

/** @sa lfModifier::ApplySubpixelGeometryDistortion */
LF_EXPORT cbool lf_modifier_apply_subpixel_geometry_distortion (
    lfModifier *modifier, float xu, float yu, int width, int height, float *res);

/** @} */

#undef cbool

#ifdef __cplusplus
}
#endif

#endif /* __LENSFUN_H__ */
