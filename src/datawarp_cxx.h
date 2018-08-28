/*
 * Copyright 2015 Cray Inc. All Rights Reserved.
 *
 * The contents of this file is proprietary information of Cray Inc.
 * and may not be disclosed without prior written consent.
 */

#include <sys/types.h>
#include <stdint.h>

enum dw_stage_type {
	DW_STAGE_IMMEDIATE,
	DW_STAGE_AT_JOB_END,
	DW_REVOKE_STAGE_AT_JOB_END,
	DW_ACTIVATE_DEFERRED_STAGE
};

/* read_failed_stage handle definition */
typedef struct dw_failed_stage_state	dw_failed_stage_t;

#ifdef __cplusplus
extern "C" {
#endif
int dw_set_stage_concurrency(const char *dw_instance_path,
			unsigned int concurrent_stages);
/*
Parameters:
* dw_instance_path:  path to the DW instance to set the concurrency value for.
  The path must exist and be readable.
* concurrent_stages:  the maximum number of concurrent stage operations 
  the datawarp instance should keep active.
Return values:
* 0: Success
* negative integer value on error, as defined in linux errno.h

The value supplied applies to the entire instance and may be silently 
constrained by the DataWarp infrastructure.  Changes to the value may 
take some amount of time to become effective.  The default, maximum 
and minimum concurrency will be configurable by the administrator.
*/

int dw_stage_file_in(const char *dw_file_path, const char *pfs_file_path);
/*
Parameters:
* dw_file_path: file path to the DW file.  The file will be created if it 
  doesn't exist or must be writable and will be truncated before the stage 
  in starts.
* pfs_path: the file path to the source file on the PFS.  The file must 
  be readable.
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Initiate an asynchronous stage in, from the PFS into the DW instance.  The 
DW infrastructure will initiate asynchronous stage operations on all DW 
nodes associated with the file.  The DW file will be fsync'd when the stage 
is complete.  Only one stage request can be active on a file at any given time.
*/

int dw_stage_directory_in(const char *dw_directory, const char *pfs_directory);
/*
Parameters:
* dw_directory: the path to the DataWarp directory.  The directory must 
  exist and be writable.
* pfs_directory: the path to the PFS directory.  All files in this directory 
  will be staged in to the dw_directory.
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Stages all files present when the call is made from pfs_directory into 
dw_directory.  Individual files are staged asynchronously.  Nested 
directories will be staged recursively.
*/

int dw_stage_list_in(const char *dw_directory, const char **pfs_list);
/*
Parameters:
* dw_directory: the path to the DataWarp directory.  The directory must 
  exist and be writable.
* pfs_list: an array (terminated by a NULL entry) of pointers to the path 
  to the PFS files (directories are not supported) to be staged.  Each file 
  in this list will be staged in to the dw_directory.  Argument may require
  a cast; see http://c-faq.com/ansi/constmismatch.html
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Stages all files present at the time the call is made from pfs_list into 
dw_directory.  Individual files are staged asynchronously.
*/

int dw_stage_file_out(const char *dw_file_path, const char *pfs_file_path,
		      enum dw_stage_type stage_type);
/*
Parameters:
* dw_file_path: the file path to the DW file to be staged out.  The file 
  must be readable.
* pfs_file_path: the path to the destination file on the PFS.  The file will 
  be created if it doesn't exist or the file must be writable and will be 
  truncated before the stage out starts.
* stage_type: The kind of stage behavior being requested:
** STAGE_IMMEDIATE: Stage the file as soon as possible
** STAGE_AT_JOB_END: Defer the stage until the job ends, either normally 
   or abnormally
** REVOKE_STAGE_AT_JOB_END: Revoke a previous STAGE_AT_JOB_END request
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Initiate an asynchronous stage out, from the DW instance into the PFS.  The 
DW infrastructure will initiate asynchronous stage operations on all DW 
nodes associated with the file.  The PFS file will be fsync'd when the 
stage is complete.    Only one stage request can be active on a file at 
any given time.

When this API returns the stage state is persistent.  When this API is 
used to swap the stage at job end state of 2 files (e.g. set stage on one 
and revoke on another) the order of the calls must be considered:
* If REVOKE_STAGE_AT_JOB_END is done first then a DW infrastructure failure 
  may cause neither file to be staged at job end
* If STAGE_AT_JOB_END is done first then a DW infrastructure failure may 
  cause both files to be staged at job end

Stage out requests active when an instance is deleted (when a job ends for 
a job instance ) will continue until they either complete successfully or 
fail.  For job instances, the job will not complete and the storage will 
not be released until the stage out finished.  Any failures of stage out 
requests in progress when the job exits will be reported to TBD.
*/

int dw_stage_directory_out(const char *dw_directory, const char *pfs_directory,
			   enum dw_stage_type stage_type);
/*
Parameters:
* dw_directory: the path to the DataWarp directory.  All files present in 
  this directory when the call is made will be staged out to the pfs_directory.
* pfs_directory: the path to the PFS directory.  The directory must exist 
  and be writable.
* stage_type: The kind of stage behavior being requested:
** STAGE_IMMEDIATE: Stage the file as soon as possible
** STAGE_AT_JOB_END: Defer the stage until the job ends, either normally 
   or abnormally
** REVOKE_STAGE_AT_JOB_END: Revoke a previous STAGE_AT_JOB_END request
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Stages all files present at the time the call is made from dw_directory 
into pfs_directory.  Individual files are staged asynchronously.  Nested 
directories will be staged recursively.

When this API returns the stage state is persistent.  When this API is used to 
swap the stage at job end state of 2 files (e.g. set stage on one and revoke 
on another) the order of the calls must be considered:
* If REVOKE_STAGE_AT_JOB_END is done first then a DW infrastructure failure 
  may cause neither file to be staged at job end
* If STAGE_AT_JOB_END is done first then a DW infrastructure failure may 
  cause both files to be staged at job end

Stage out requests active when an instance is deleted (when a job ends for 
a job instance) will continue until they either complete successfully 
or fail. For job instances, the job will not complete and the storage will 
not be released until the stage out is complete. Any failures of stage 
out requests in progress when the job exits will be reported to TBD.
*/

int dw_stage_list_out(const char **dw_list, const char *pfs_directory,
		      enum dw_stage_type stage_type);
/*
Parameters:
* dw_list: an array (terminated by a NULL entry) of pointers to the paths 
  to the DW files (directories are not supported) to be staged.  Each file 
  in this list will be staged out to the pfs_directory.  Argument may require
  a cast; see http://c-faq.com/ansi/constmismatch.html
* pfs_directory: the path to the PFS directory.  The directory must exist 
  and be writable.
* stage_type: The kind of stage behavior being requested:
** STAGE_IMMEDIATE: Stage the file as soon as possible
** STAGE_AT_JOB_END: Defer the stage until the job ends, either normally 
   or abnormally
** REVOKE_STAGE_AT_JOB_END: Revoke a previous STAGE_AT_JOB_END request
Return values:
* 0: success
* negative integer value on error, as defined in linux errno.h

Stages all files present at the time the call is made from dw_list into 
pfs_directory.  Individual files are staged asynchronously.

When this API returns the stage state is persistent.  When this API is 
used swap the stage at job end state of 2 files (e.g. set stage on one 
and revoke on another) the order of the calls must be considered:
* If REVOKE_STAGE_AT_JOB_END is done first then a DW infrastructure failure 
  may cause neither file to be staged at job end
* If STAGE_AT_JOB_END is done first then a DW infrastructure failure may 
  cause both files to be staged at job end

Stage out requests active when an instance is deleted (when a job ends for 
a job instance) will continue until they either complete successfully or 
fail. For job instances, the job will not complete and the storage will not 
be released until the stage out is complete. Any failures of stage out 
requests in progress when the job exits will be reported to TBD.
*/

int dw_query_file_stage(const char *dw_file_path, int *complete, int *pending,
			int *deferred, int *failed);
/*
Parameters:
* dw_file_path: DW file to query
* complete: the number of completed stage operations
* pending:  the number of pending (active and waiting) stage operations
* deferred: the number of deferred stage operations
* failed: the number of stage operations that have failed
Return values:
* 0; success
* -ENOENT: specified file does not exist
* -EINVAL: the specified file has not had a stage operation initiated on it
* negative integer value on error, as defined in linux errno.h

For a single file query only one of the returned counts will be non-zero.
*/

int dw_query_directory_stage(const char *dw_directory_path, int *complete,
			     int *pending, int *deferred, int *failed);
/*
Parameters:
* dw_directory_path: DW directory to query
* complete: the number of completed stage operations
* pending:  the number of pending (active and waiting) stage operations
* deferred: the number of deferred stage operations
* failed: the number of stage operations that have failed
Return values:
* 0; success
* -ENOENT: specified directory does not exist
* negative integer value on error, as defined in linux errno.h
*/

int dw_query_list_stage(const char **dw_list_path, int *complete, int *pending,
			int *deferred, int *failed);
/*
Parameters:
* dw_list_path: pointer to a list of DW files to query
* complete: the number of completed stage operations
* pending:  the number of pending (active and waiting) stage operations
* deferred: the number of deferred stage operations
* failed: the number of stage operations that have failed
Return values:
* 0; success
* -ENOENT: specified file does not exist
* -EINVAL: a specified file has not had a stage operation initiated on it
* negative integer value on error, as defined in linux errno.h
*/

int dw_open_failed_stage(const char *dw_instance_path,
			dw_failed_stage_t **handle);

int dw_read_failed_stage(dw_failed_stage_t *handle, char *path,
			int path_size, int *ret_errno);

int dw_close_failed_stage(dw_failed_stage_t *handle);
/*
Parameters:
* dw_instance_path: DW directory path to begin file tree walk from
* path: pointer to memory to receive the DataWarp path name of a failed stage
* path_size: maximum size of the path to be returned to the path parameter
* ret_errno: Errno reason that the stage failed
* handle: value returned from a prior open_failed_stage
Return values:
*open_failed_stage:
** ==0: success, pointer to newly created dw_failed_stage_t handle for use in
   read_failed_stage and close_failed_stage returned in handle argument 
** <0:  negative errno value, as defined in linux errno.h.  handle is not valid
*read_failed_stage:
** >0: path and ret_errno returned successfully.  Return value is length of 
   returned path.  ret_errno is -errno.
** ==0: completed path walk, no more failed stages
** <0: error, negative errno value returned as defined in linux errno.h
*close_failed_stage:
** ==0: success, handle has been cleaned up
** <0: error, negative errno value returned as defined in linux errno.h

read_failed_stage returns the DataWarp path of a stage operation that has 
failed.  It should be called multiple times to discover all failed stage 
operations.  A file remains a failed stage until the file is either unlinked, 
the stage is terminated or the stage is restarted.  To restart a failed 
stage scan, the handle must be closed and reopened (e.g. there is no 
equivalent of lseek).

If the list of failed stages changes asynchronously after an open_failed_stage 
is started those changes may not be visible to that instance of 
open_failed_stage.  Changes made to a failed stage already seen by an 
instance of open_failed_stage (for example terminating or restarting a 
stage or unlinking the file) will not affect that instance.

Example:
 dw_open_failed_stage("dw_instance_path", &hdl);
 rval = dw_read_failed_stage(hdl, &buf, 1024, &errval);
 while (rval > 0) {
    printf("Stage failed on: %s, errno: %d\n", buf, errval);
    rval = dw_read_failed_stage(hdl, &buf, 1024, &errval);
 }
 dw_close_failed_stage(hdl);
*/

int dw_wait_file_stage(const char *dw_file_path);
/*
Parameters:
* dw_file_path: DW file to wait for
Return values:
* 0; success
* -ENOENT: specified file does not exist
* -EINVAL: the specified file has not had a stage operation initiated on it
* negative integer value on error, as defined in linux errno.h

Wait for a stage operation to complete on the target file.  The calling process
is blocked until the stage is complete, this request is not interruptible by 
a signal.
*/

int dw_wait_directory_stage(const char *dw_directory_path);
/*
Parameters:
* dw_directory_path: DW directory containing files to wait for
Return values:
* 0; success
* -ENOENT: specified directory does not exist
* negative integer value on error, as defined in linux errno.h

Wait for one or all stage operations to complete.  The calling process is 
blocked until the stage is complete, this request is not interruptible by a 
signal.
*/

int dw_wait_list_stage(const char **dw_list);
/*
Parameters:
* dw_list: list of DW files to wait for.  Argument may require a cast;
  see http://c-faq.com/ansi/constmismatch.html
Return values:
* 0; success
* -ENOENT: a specified file does not exist
* -EINVAL: the specified file has not had a stage operation initiated on it
* negative integer value on error, as defined in linux errno.h

Wait for one or all stage operations to complete.  The calling process is 
blocked until the stage is complete, this request is not interruptible by a 
signal.
*/

int dw_terminate_file_stage(const char *dw_file_path);
/*
Parameters:
* dw_file_path: DW file to terminate the stage
Return values:
* 0; success
* -ENOENT: specified file does not exist
* -EINVAL: the specified file has not had a stage operation initiated on it

Terminate an in progress or waiting stage operation.  If the stage is in 
progress the amount of data written by the stage is undefined.
*/

int dw_terminate_directory_stage(const char *dw_directory_path);
/*
Parameters:
* dw_directory_path: DW directory of files to terminate stages
Return values:
* 0; success
* -ENOENT: specified directory does not exist
* -EINVAL: the specified directory has not had a stage operation initiated on it

Terminate one or more in progress or waiting stage operations.  If the stage 
is in progress the amount of data written by the stage is undefined.
*/

int dw_terminate_list_stage(const char **dw_list);
/*
Parameters:
* dw_list: pointer to a list of DW files to terminate.  Argument may require a
  cast; see http://c-faq.com/ansi/constmismatch.html
Return values:
* 0; success
* -ENOENT: specified file does not exist
* -EINVAL: the specified file has not had a stage operation initiated on it
* negative integer value on error, as defined in linux errno.h

Terminate one or more in progress or waiting stage operations.  If the stage 
is in progress the amount of data written by the stage is undefined.
*/

int dw_set_stripe_configuration(int fd, int stripe_size, int stripe_width);
/*
Parameters:
* fd:  open file descriptor of the file or directory to set the stripe 
  configuration of
* stripe_size: the size of a stripe, in bytes
* stripe_width: the number of stripes to use

Errors:
* -EINVAL:  stripe_width is invalid (0 or too large)
* -EINVAL:  stripe_size is invalid (0 or not a multiple of 4k)
* negative integer value on error, as defined in linux errno.h

set_stripe_configuration sets the current stripe configuration for a file or 
directory.  The behavior after changing the stripe configuration of a file 
that has data in it is undefined.  The stripe configuration of a file is 
persistent and can be queried via get_stripe_configuration.  Opening a file 
that has had a set_stripe_configuration done on it will use the stripe 
configuration associated with the file.

If the stripe configuration is set on a directory then all files created in 
that directory after that point in time will use the stripe configuration 
from the directory as their default stripe configuration, the stripe 
configuration of existing files does not change. Directories do not inherit 
the stripe configuration of their parent directory.

The stripe_size should be a multiple of the instance default stripe size to 
avoid wasting space due to a mismatch with the storage configuration 
underlying the DataWarp instance.
*/

int dw_get_stripe_configuration(int fd, int *stripe_size, int *stripe_width,
   int *starting_index);
/*
Parameters:
* fd:  open file descriptor of the file or directory to be queried
* stripe_size: pointer to an int to receive the size of a stripe
* stripe_width: pointer to an int to receive the number of stripes used 
* starting_index: pointer to an int to receive the stripe index (between 0 and stripe_width) 
  for file offset 0

Errors:
* negative integer value on error, as defined in linux errno.h

get_stripe_configuration returns the current stripe configuration for a file.  
The stripe_width represents the maximum number of stripes the file can have, 
not the current number it is actually using.  For a file that has no stripe 
configuration, get_stripe_configuration returns the default values for the 
DW type and access mode of the file.
*/

char *dw_get_mds_path(const char *dw_root, uint64_t key);
/*
Parameters:
* dw_root: The root path to a striped DW mount to get the metadata server (MDS)
  path for.  For example $DW_JOB_STRIPED.
* key: An application provided value that the DataWarp infrastructure will use
  to determine which DataWarp node within an instance to use as the instance
  MDS for that key. The same value of key will map to the same MDS regardless
  of what process or node it comes from. If multiple MDS servers were not
  requested on the instance then the path to the default MDS server is
  returned. The default MDS server is used by job script/WLM orchestrated
  stage in.

Errors:
* NULL: invalid dw_root, or generated mount point not found

Return:
A pointer to a string containing the path prefix the application should use
when referencing files associated with the input key. This string should be
prepended to the remainder of the application provided file path similar to
the way $DW_JOB_STRIPED would be used.  This string is allocated by the API
and the caller is responsible for freeing it.
*/

char *dw_get_accounting_data_json(const char *path, int *ecode);
/*
Parameters:
* path: datawarp path
* ecode: error code pointer
Errors:
* NULL: invalid data
Return:
accounting data in json.  This json string is allocated by the API and
the caller is responsible for freeing it.
Note for json result string:
"realm", "fragments", and "namespaces" keys are used for objects containing group of
accounting data. "fragments" and "namespaces" are arrays.
Example output:
{
  "realm": {
    "namespace count": 2,
    "realm id": 50050014,
    "fragments": [
      {
        "lower filesystem capacity": 3209691136,
        "cur capacity used": 34119680,
        "max window write": 86400,
        "max capacity used": 34119680,
        "num files created": 0,
        "write highwater": 0,
        "moving avg writes": 0,
        "write limit": 32212254720,
        "bytes written": 0,
        "bytes read": 0,
        "server name": "datawarp12-s2"
      }
    ],
    "namespaces": [
      {
        "namespace id": 17,
        "stripe width": 1,
        "stripe size": 8388608,
        "substripe width": 12,
        "substripe size": 8388608,
        "fs bytes read": 0,
        "num files create threshold": 0,
        "fs bytes written": 0,
        "file size limit": 0,
        "num files created": 0,
        "max offset written": 0,
        "stage bytes read": 0,
        "max offset read": 0,
        "stage bytes written": 0
      },
      {
        "namespace id": 18,
        "stripe width": 1,
        "stripe size": 8388608,
        "substripe width": 12,
        "substripe size": 8388608,
        "fs bytes read": 0,
        "num files create threshold": 0,
        "fs bytes written": 0,
        "file size limit": 0,
        "num files created": 0,
        "max offset written": 0,
        "stage bytes read": 0,
        "max offset read": 0,
        "stage bytes written": 0
      }
    ],
    "server count": 1
  }
}
*/
#ifdef __cplusplus
}
#endif
