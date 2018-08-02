/*-------------------------------------------------------------------------
 *
 * cdbpath.h
 *
 *
 * Portions Copyright (c) 2005-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/include/cdb/cdbpath.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CDBPATH_H
#define CDBPATH_H

#include "nodes/relation.h"

void
cdbpath_cost_motion(PlannerInfo *root, CdbMotionPath *motionpath);

Path *
cdbpath_create_motion_path(PlannerInfo     *root,
                           Path            *subpath,
                           List            *pathkeys,
                           bool             require_existing_order,
                           CdbPathLocus     locus);

CdbPathLocus
cdbpath_motion_for_join(PlannerInfo    *root,
                        JoinType        jointype,           /* JOIN_INNER/FULL/LEFT/RIGHT/IN */
                        Path          **p_outer_path,       /* INOUT */
                        Path          **p_inner_path,       /* INOUT */
                        List           *mergeclause_list,   /* equijoin RestrictInfo list */
                        List           *outer_pathkeys,
                        List           *inner_pathkeys,
                        bool            outer_require_existing_order,
                        bool            inner_require_existing_order);

void 
cdbpath_dedup_fixup(PlannerInfo *root, Path *path);

bool
cdbpath_contains_wts(Path *path);

/*
 * cdbpath_rows
 *
 * Returns a Path's estimated number of result rows.
 */
static inline double
cdbpath_rows(PlannerInfo *root, Path *path)
{
	/* GPDB_92_MERGE_FIXME: Maybe we should think about removing this function.
	 * That will eliminate merge risk since pg upstream (since 9.2) uses
	 * path->rows directly.
	 */
	Path  *p;

	p = (IsA(path, CdbMotionPath))  ? ((CdbMotionPath *)path)->subpath
		: path;

	if (IsA(p, BitmapHeapPath) ||
			IsA(p, BitmapAppendOnlyPath) ||
			IsA(p, IndexPath) ||
			IsA(p, UniquePath))
		return p->rows;

	if (CdbPathLocus_IsReplicated(path->locus))
		return  (path->parent->rows * root->config->cdbpath_segments);

	return  path->parent->rows;
}                               /* cdbpath_rows */

#endif   /* CDBPATH_H */
