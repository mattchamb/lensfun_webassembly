/*
    Auxiliary helper functions for Lensfun
    Copyright (C) 2007 by Andrew Zabolotny
*/

#include "config.h"
#include "lensfun.h"
#include "lensfunprv.h"
#include <locale.h>
#include <ctype.h>
#include <math.h>

typedef char gchar;

static const char *_lf_get_lang ()
{
    static char lang [16];
    return strcpy (lang, "en");
}

void lf_free (void *data)
{
    free (data);
}

const char *lf_mlstr_get (const lfMLstr str)
{
    if (!str)
        return str;

    /* Get the current locale for messages */
    const char *lang = _lf_get_lang ();
    /* Default value if no language matches */
    const char *def = str;
    /* Find the corresponding string in the lot */
    const char *cur = strchr (str, 0) + 1;
    while (*cur)
    {
        const char *next = strchr (cur, 0) + 1;
        if (!strcmp (cur, lang))
            return next;
        if (!strcmp (cur, "en"))
            def = next;
        if (*(cur = next))
            cur = strchr (cur, 0) + 1;
    }

    return def;
}

lfMLstr lf_mlstr_add (lfMLstr str, const char *lang, const char *trstr)
{
    if (!trstr)
        return str;
    size_t trstr_len = strlen (trstr) + 1;

    /* Find the length of multi-language string */
    size_t str_len = 0;
    if (str)
    {
        str_len = strlen (str) + 1;
        while (str [str_len])
            str_len += 1 + strlen (str + str_len);
    }

    if (!lang)
    {
        /* Replace the default string */
        size_t def_str_len = str ? strlen (str) + 1 : 0;

        memcpy (str + trstr_len, str + def_str_len, str_len - def_str_len);

        str = (char *)realloc (str, str_len - def_str_len + trstr_len + 1);

        memcpy (str, trstr, trstr_len);
        str_len = str_len - def_str_len + trstr_len;
        str [str_len] = 0;

        return str;
    }

    size_t lang_len = lang ? strlen (lang) + 1 : 0;

    str = (char *)realloc (str, str_len + lang_len + trstr_len + 1);
    memcpy (str + str_len, lang, lang_len);
    memcpy (str + str_len + lang_len, trstr, trstr_len);
    str [str_len + lang_len + trstr_len] = 0;

    return str;
}

lfMLstr lf_mlstr_dup (const lfMLstr str)
{
    /* Find the length of multi-language string */
    size_t str_len = 0;
    if (str)
    {
        str_len = strlen (str) + 1;
        while (str [str_len])
            str_len += 2 + strlen (str + str_len + 1);
        /* Reserve space for the last - closing - zero */
        str_len++;
    }

    gchar *ret = (char *)malloc (str_len);
    memcpy (ret, str, str_len);
    return ret;
}

void _lf_list_free (void **list)
{
    int i;

    if (!list)
        return;

    for (i = 0; list [i]; i++)
        free (list [i]);

    free (list);
}

void _lf_setstr (gchar **var, const gchar *val)
{
    if (*var)
        free (*var);

    *var = strdup (val);
}

void _lf_addstr (gchar ***var, const gchar *val)
{
    _lf_addobj ((void ***)var, val, strlen (val) + 1, NULL);
}

void _lf_addobj (void ***var, const void *val, size_t val_size,
                 bool (*cmpf) (const void *, const void *))
{
    int n = 0;

    if (*var)
        for (n = 0; (*var) [n]; n++)
            if (cmpf && cmpf (val, (*var) [n]))
            {
                free ((*var) [n]);
                goto alloc_copy;
                return;
            }

    n++;

    (*var) = (void **)realloc (*var, (n + 1) * sizeof (void *));
    (*var) [n--] = NULL;

alloc_copy:
    (*var) [n] = malloc (val_size);
    memcpy ((*var) [n], val, val_size);
}

bool _lf_delobj (void ***var, int idx)
{
    if (!(*var))
        return false;

    int len;
    for (len = 0; (*var) [len]; len++)
        ;
    if (idx < 0 || idx >= len)
        return false;

    free ((*var) [idx]);
    memmove (&(*var) [idx], &(*var) [idx + 1], (len - idx) * sizeof (void *));
    (*var) = (void **)realloc (*var, len * sizeof (void *));
    return true;
}

float _lf_interpolate (float y1, float y2, float y3, float y4, float t)
{
    float tg2, tg3;
    float t2 = t * t;
    float t3 = t2 * t;

    if (y1 == FLT_MAX)
        tg2 = y3 - y2;
    else
        tg2 = (y3 - y1) * 0.5;

    if (y4 == FLT_MAX)
        tg3 = y3 - y2;
    else
        tg3 = (y4 - y2) * 0.5;

    // Hermite polynomial
    return (2 * t3 - 3 * t2 + 1) * y2 +
        (t3 - 2 * t2 + t) * tg2 +
        (-2 * t3 + 3 * t2) * y3 +
        (t3 - t2) * tg3;
}
