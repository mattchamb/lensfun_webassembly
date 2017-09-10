/*
    Private constructors and destructors
    Copyright (C) 2007 by Andrew Zabolotny
*/

#include "config.h"
#include "lensfun.h"
#include "lensfunprv.h"

lfCamera::lfCamera ()
{
    // Defaults for attributes are "unknown" (mostly 0).  Otherwise, ad hoc
    // lfLens instances used for searches could not be matched against database
    // lenses easily.  If you need defaults for database tags, set them when
    // reading the database.
    memset (this, 0, sizeof (*this));
}

lfCamera::~lfCamera ()
{
    lf_free (Maker);
    lf_free (Model);
    lf_free (Variant);
    lf_free (Mount);
}

void lfCamera::SetMaker (const char *val, const char *lang)
{
    Maker = lf_mlstr_add (Maker, lang, val);
}

void lfCamera::SetModel (const char *val, const char *lang)
{
    Model = lf_mlstr_add (Model, lang, val);
}

void lfCamera::SetVariant (const char *val, const char *lang)
{
    Variant = lf_mlstr_add (Variant, lang, val);
}

void lfCamera::SetMount (const char *val)
{
    _lf_setstr (&Mount, val);
}

bool lfCamera::Check ()
{
    if (!Maker || !Model || !Mount || CropFactor <= 0)
        return false;

    return true;
}

//---------------------------// The C interface //---------------------------//

lfCamera *lf_camera_new ()
{
    return new lfCamera ();
}

void lf_camera_destroy (lfCamera *camera)
{
    delete camera;
}

cbool lf_camera_check (lfCamera *camera)
{
    return camera->Check ();
}

int _lf_camera_compare (const void* a, const void* b)
{
    lfCamera *i1 = (lfCamera *)a;
    lfCamera *i2 = (lfCamera *)b;

    int cmp = _lf_strcmp (i1->Maker, i2->Maker);
    if (cmp != 0)
        return cmp;

    cmp = _lf_strcmp (i1->Model, i2->Model);
    if (cmp != 0)
        return cmp;

    return _lf_strcmp (i1->Variant, i2->Variant);
}
