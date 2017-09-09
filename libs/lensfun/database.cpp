/*
    Lens database methods implementation
    Copyright (C) 2007 by Andrew Zabolotny
*/

#include "config.h"
#include "lensfun.h"
#include "lensfunprv.h"
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>
#include <fstream>
#include "windows/mathconstants.h"

#ifdef PLATFORM_WINDOWS
#  include <io.h>
#else
#  include <unistd.h>
#endif

lfDatabase::lfDatabase ()
{

}

lfDatabase::~lfDatabase ()
{

}

// static int __find_camera_compare (void* a, void* b)
// {
//     lfCamera *i1 = (lfCamera *)a;
//     lfCamera *i2 = (lfCamera *)b;

//     if (i1->Maker && i2->Maker)
//     {
//         int cmp = _lf_strcmp (i1->Maker, i2->Maker);
//         if (cmp != 0)
//             return cmp;
//     }

//     if (i1->Model && i2->Model)
//         return _lf_strcmp (i1->Model, i2->Model);

//     return 0;
// }

// const lfCamera **lfDatabase::FindCameras (const char *maker, const char *model) const
// {
//     if (maker && !*maker)
//         maker = NULL;
//     if (model && !*model)
//         model = NULL;

//     lfCamera tc;
//     tc.SetMaker (maker);
//     tc.SetModel (model);
//     int idx = _lf_ptr_array_find_sorted ((GPtrArray *)Cameras, &tc, __find_camera_compare);
//     if (idx < 0)
//         return NULL;

//     guint idx1 = idx;
//     while (idx1 > 0 &&
//            __find_camera_compare (g_ptr_array_index ((GPtrArray *)Cameras, idx1 - 1), &tc) == 0)
//         idx1--;

//     guint idx2 = idx;
//     while (++idx2 < ((GPtrArray *)Cameras)->len - 1 &&
//            __find_camera_compare (g_ptr_array_index ((GPtrArray *)Cameras, idx2), &tc) == 0)
//         ;

//     const lfCamera **ret = g_new (const lfCamera *, idx2 - idx1 + 1);
//     for (guint i = idx1; i < idx2; i++)
//         ret [i - idx1] = (lfCamera *)g_ptr_array_index ((GPtrArray *)Cameras, i);
//     ret [idx2 - idx1] = NULL;
//     return ret;
// }

// static gint _lf_compare_camera_score (gconstpointer a, gconstpointer b)
// {
//     lfCamera *i1 = (lfCamera *)a;
//     lfCamera *i2 = (lfCamera *)b;

//     return i2->Score - i1->Score;
// }

// const lfCamera **lfDatabase::FindCamerasExt (const char *maker, const char *model,
//                                              int sflags) const
// {
//     if (maker && !*maker)
//         maker = NULL;
//     if (model && !*model)
//         model = NULL;

//     GPtrArray *ret = g_ptr_array_new ();

//     lfFuzzyStrCmp fcmaker (maker, (sflags & LF_SEARCH_LOOSE) == 0);
//     lfFuzzyStrCmp fcmodel (model, (sflags & LF_SEARCH_LOOSE) == 0);

//     for (size_t i = 0; i < ((GPtrArray *)Cameras)->len - 1; i++)
//     {
//         lfCamera *dbcam = static_cast<lfCamera *> (g_ptr_array_index ((GPtrArray *)Cameras, i));
//         int score1 = 0, score2 = 0;
//         if ((!maker || (score1 = fcmaker.Compare (dbcam->Maker))) &&
//             (!model || (score2 = fcmodel.Compare (dbcam->Model))))
//         {
//             dbcam->Score = score1 + score2;
//             _lf_ptr_array_insert_sorted (ret, dbcam, _lf_compare_camera_score);
//         }
//     }

//     // Add a NULL to mark termination of the array
//     if (ret->len)
//         g_ptr_array_add (ret, NULL);

//     // Free the GPtrArray but not the actual list.
//     return (const lfCamera **) (g_ptr_array_free (ret, FALSE));
// }

// const lfCamera *const *lfDatabase::GetCameras () const
// {
//     return (lfCamera **)((GPtrArray *)Cameras)->pdata;
// }

// const lfLens **lfDatabase::FindLenses (const lfCamera *camera,
//                                        const char *maker, const char *model,
//                                        int sflags) const
// {
//     if (maker && !*maker)
//         maker = NULL;
//     if (model && !*model)
//         model = NULL;

//     lfLens lens;
//     lens.SetMaker (maker);
//     lens.SetModel (model);
//     if (camera)
//         lens.AddMount (camera->Mount);
//     // Guess lens parameters from lens model name
//     lens.GuessParameters ();
//     lens.CropFactor = camera ? camera->CropFactor : 0.0;
//     return FindLenses (&lens, sflags);
// }

// static gint _lf_compare_lens_score (gconstpointer a, gconstpointer b)
// {
//     lfLens *i1 = (lfLens *)a;
//     lfLens *i2 = (lfLens *)b;

//     return i2->Score - i1->Score;
// }

// static gint _lf_compare_lens_details (gconstpointer a, gconstpointer b)
// {
//     // Actually, we not only sort by focal length, but by MinFocal, MaxFocal,
//     // MinAperature, Maker, and Model -- in this order of priorities.
//     lfLens *i1 = (lfLens *)a;
//     lfLens *i2 = (lfLens *)b;

//     int cmp = _lf_lens_parameters_compare (i1, i2);
//     if (cmp != 0)
//         return cmp;

//     return _lf_lens_name_compare (i1, i2);
// }

// static void _lf_add_compat_mounts (
//     const lfDatabase *This, const lfLens *lens, GPtrArray *mounts, char *mount)
// {
//     const lfMount *m = This->FindMount (mount);
//     if (m && m->Compat)
//         for (int i = 0; m->Compat [i]; i++)
//         {
//             mount = m->Compat [i];

//             int idx = _lf_ptr_array_find_sorted (mounts, mount, (GCompareFunc)_lf_strcmp);
//             if (idx >= 0)
//                 continue; // mount already in the list

//             // Check if the mount is not already in the main list
//             bool already = false;
//             for (int j = 0; lens->Mounts [j]; j++)
//                 if (!_lf_strcmp (mount, lens->Mounts [j]))
//                 {
//                     already = true;
//                     break;
//                 }
//             if (!already)
//                 _lf_ptr_array_insert_sorted (mounts, mount, (GCompareFunc)_lf_strcmp);
//         }
// }

// const lfLens **lfDatabase::FindLenses (const lfLens *lens, int sflags) const
// {
//     GPtrArray *ret = g_ptr_array_new ();
//     GPtrArray *mounts = g_ptr_array_new ();

//     lfFuzzyStrCmp fc (lens->Model, (sflags & LF_SEARCH_LOOSE) == 0);

//     // Create a list of compatible mounts
//     if (lens->Mounts)
//         for (int i = 0; lens->Mounts [i]; i++)
//             _lf_add_compat_mounts (this, lens, mounts, lens->Mounts [i]);
//     g_ptr_array_add (mounts, NULL);

//     int score;
//     const bool sort_and_uniquify = (sflags & LF_SEARCH_SORT_AND_UNIQUIFY) != 0;
//     for (size_t i = 0; i < ((GPtrArray *)Lenses)->len - 1; i++)
//     {
//         lfLens *dblens = static_cast<lfLens *> (g_ptr_array_index ((GPtrArray *)Lenses, i));
//         if ((score = _lf_lens_compare_score (
//             lens, dblens, &fc, (const char **)mounts->pdata)) > 0)
//         {
//             dblens->Score = score;
//             if (sort_and_uniquify) {
//                 bool already = false;
//                 for (size_t i = 0; i < ret->len; i++)
//                 {
//                     const lfLens *previous_lens = static_cast<lfLens *> (g_ptr_array_index (ret, i));
//                     if (!_lf_lens_name_compare (previous_lens, dblens))
//                     {
//                         if (dblens->Score > previous_lens->Score)
//                             ret->pdata[i] = dblens;
//                         already = true;
//                         break;
//                     }
//                 }
//                 if (!already)
//                     _lf_ptr_array_insert_sorted (ret, dblens, _lf_compare_lens_details);
//             }
//             else
//                 _lf_ptr_array_insert_sorted (ret, dblens, _lf_compare_lens_score);
//         }
//     }

//     // Add a NULL to mark termination of the array
//     if (ret->len)
//         g_ptr_array_add (ret, NULL);

//     g_ptr_array_free (mounts, TRUE);

//     // Free the GPtrArray but not the actual list.
//     return (const lfLens **) (g_ptr_array_free (ret, FALSE));
// }

// const lfLens *const *lfDatabase::GetLenses () const
// {
//     return (lfLens **)((GPtrArray *)Lenses)->pdata;
// }

// const lfMount *lfDatabase::FindMount (const char *mount) const
// {
//     lfMount tm;
//     tm.SetName (mount);
//     int idx = _lf_ptr_array_find_sorted ((GPtrArray *)Mounts, &tm, _lf_mount_compare);
//     if (idx < 0)
//         return NULL;

//     return (const lfMount *)g_ptr_array_index ((GPtrArray *)Mounts, idx);
// }

// const char *lfDatabase::MountName (const char *mount) const
// {
//     const lfMount *m = FindMount (mount);
//     if (!m)
//         return mount;
//     return lf_mlstr_get (m->Name);
// }

// const lfMount * const *lfDatabase::GetMounts () const
// {
//     return (lfMount **)((GPtrArray *)Mounts)->pdata;
// }

// lfDatabase *lf_db_new ()
// {
//     return new lfDatabase ();
// }

// void lf_db_destroy (lfDatabase *db)
// {
//     db->Destroy ();
// }

// long int lf_db_read_timestamp (const char *dirname)
// {
//     return lfDatabase::ReadTimestamp(dirname);
// }

// lfError lf_db_load (lfDatabase *db)
// {
//     return db->Load ();
// }

// lfError lf_db_load_file (lfDatabase *db, const char *filename)
// {
//     return db->Load (filename);
// }

// cbool lf_db_load_directory (lfDatabase *db, const char *dirname)
// {
//     return db->Load (dirname);
// }

// lfError lf_db_load_path (lfDatabase *db, const char *pathname)
// {
//     return db->Load (pathname);
// }

// lfError lf_db_load_data (lfDatabase *db, const char *errcontext,
//                          const char *data, size_t data_size)
// {
//     return db->Load (errcontext, data, data_size);
// }

// lfError lf_db_save_all (const lfDatabase *db, const char *filename)
// {
//     return db->Save (filename);
// }

// lfError lf_db_save_file (const lfDatabase *db, const char *filename,
//                          const lfMount *const *mounts,
//                          const lfCamera *const *cameras,
//                          const lfLens *const *lenses)
// {
//     return db->Save (filename, mounts, cameras, lenses);
// }

// char *lf_db_save (const lfMount *const *mounts,
//                   const lfCamera *const *cameras,
//                   const lfLens *const *lenses)
// {
//     return lfDatabase::Save (mounts, cameras, lenses);
// }

// const lfCamera **lf_db_find_cameras (const lfDatabase *db,
//                                      const char *maker, const char *model)
// {
//     return db->FindCameras (maker, model);
// }

// const lfCamera **lf_db_find_cameras_ext (
//     const lfDatabase *db, const char *maker, const char *model, int sflags)
// {
//     return db->FindCamerasExt (maker, model, sflags);
// }

// const lfCamera *const *lf_db_get_cameras (const lfDatabase *db)
// {
//     return db->GetCameras ();
// }

// const lfLens **lf_db_find_lenses_hd (const lfDatabase *db, const lfCamera *camera,
//                                      const char *maker, const char *lens, int sflags)
// {
//     return db->FindLenses (camera, maker, lens, sflags);
// }

// const lfLens **lf_db_find_lenses (const lfDatabase *db, const lfLens *lens, int sflags)
// {
//     return db->FindLenses (lens, sflags);
// }

// const lfLens *const *lf_db_get_lenses (const lfDatabase *db)
// {
//     return db->GetLenses ();
// }

// const lfMount *lf_db_find_mount (const lfDatabase *db, const char *mount)
// {
//     return db->FindMount (mount);
// }

// const char *lf_db_mount_name (const lfDatabase *db, const char *mount)
// {
//     return db->MountName (mount);
// }

// const lfMount * const *lf_db_get_mounts (const lfDatabase *db)
// {
//     return db->GetMounts ();
// }
