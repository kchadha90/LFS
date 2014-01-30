/*
 *************************************************************************
 *
 * disk.h --
 *
 *	Declarations for the Disk layer. 
 * 
 *      NOTE: The Disk layer is not thread safe. If your LFS is multi-threaded
 *      you'll have to put the proper synchronization functions around your
 *      calls to these routines.
 *
 *
 *************************************************************************
 */

#ifndef _DISK_H
#define _DISK_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *************************************************************************
 *
 * Useful definitions.
 *
 *************************************************************************
 */

typedef int Boolean;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 *************************************************************************
 *
 * The following are flags to pass to Disk_Open.
 *
 * DISK_SILENT	don't print statistics when Disk_Close is called.
 * DISK_ASYNC	don't simulate synchronous disk operations. By default
 *		Disk delays for 10ms on every read and write to simulate
 *		disk access times.
 *
 *************************************************************************
 */

typedef u_int Disk_Flags;

#define DISK_SILENT	0x1	
#define DISK_ASYNC	0x2	

/*
 *************************************************************************
 *
 * Disk sector size. Don't change this, I haven't tested with other values.
 *
 *************************************************************************
 */

#define DISK_SECTOR_SIZE 512

/*
 *************************************************************************
 *
 * Handle for a disk returned by Disk_Open and passed to the other routines.
 *
 *************************************************************************
 */

typedef void *Disk;

/*
 *************************************************************************
 * Disk
 * Disk_Open
 *
 * Parameters:
 *
 *  	char 		*file	-- name of the disk file to open.
 *  	Disk_Flags	flags	-- see above
 *  	u_int		*sectors -- # of sectors in the disk
 *
 * Returns:
 *	Disk handle on success, NULL otherwise and errno is set.
 *
 *
 * Disk_Open opens a disk and returns a handle for it that is used
 * in subsequent calls to Disk_Read, Disk_Write, and Disk_Close. 
 * The specified file must exist. The disk size in sectors is returned
 * in "sectors".
 *
 *************************************************************************
 */

Disk	Disk_Open(char *file, Disk_Flags flags, u_int *sectors);

/*
 *************************************************************************
 * int
 * Disk_Read
 *
 * Parameters:
 *
 *  	Disk 		disk	-- disk to read
 *	u_int		offset -- starting offset, in sectors
 * 	u_int		count -- # of sectors to read
 *	void		*buffer -- buffer into which disk is read
 *
 * Returns:
 *	0 on success, 1 otherwise and errno is set.
 *
 *
 * Disk_Read reads "count" sectors from "disk" into "buffer" 
 * starting at sector "offset". 
 *
 *************************************************************************
 */

int	Disk_Read(Disk disk, u_int offset, u_int count, void *buffer);

/*
 *************************************************************************
 * int
 * Disk_Write
 *
 * Parameters:
 *
 *  	Disk 		disk	-- disk to write
 *	u_int		offset -- starting offset, in sectors
 * 	u_int		count -- # of sectors to write
 *	void		*buffer -- buffer from which disk is written
 *
 * Returns:
 *	0 on success, 1 otherwise and errno is set.
 *
 *
 * Disk_Write writes "count" sectors from "buffer" to "disk" 
 * starting at sector "offset". 
 *
 *************************************************************************
 */

int	Disk_Write(Disk disk, u_int offset, u_int count, void *buffer);

/*
 *************************************************************************
 * int
 * Disk_Close
 *
 * Parameters:
 *
 *  	Disk 		disk	-- disk to close
 *
 * Returns:
 *	0 on success, 1 otherwise and errno is set.
 *
 *
 * Disk_Close closes the disk.
 *
 *************************************************************************
 */

int	Disk_Close(Disk disk);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
