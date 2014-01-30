#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "disk.h"

#define assume(expr) {                                                  \
    if (!(expr)) {                                                      \
        printf("%s:%d: failed assumption `%s'\n", __FILE__, __LINE__,   \
            #expr);                                                     \
        abort();                                                        \
    }                                                                   \
}

#define DISK_READ 0
#define DISK_WRITE 1

typedef struct DiskInfo {
    char	*file;
    Disk_Flags	flags;
    int		fd;
    int         sectors;
    long long	readOps;
    long long	readSectors;
    long long	writeOps;
    long long	writeSectors;
} DiskInfo;

Disk
Disk_Open(
    char	*file,
    Disk_Flags	flags,
    u_int	*sectors)
{
    DiskInfo	*disk;
    struct stat	buf;
    int		rc;

    fprintf(stderr, "In Disk_Open\n");

    disk = (DiskInfo *) malloc(sizeof(DiskInfo));
    memset(disk, 0, sizeof(*disk));
    assume(disk != NULL);
    disk->file = file;
    disk->flags = flags;
    disk->fd = open(file, O_RDWR);
    if (disk->fd < 0) {
	rc = 1;
	goto done;
    }
    rc = fstat(disk->fd, &buf);
    if (rc != 0) {
	goto done;
    }
    disk->sectors = buf.st_size / DISK_SECTOR_SIZE;
    *sectors = disk->sectors;
    rc = 0;
done:
    if (rc) {
	free((void *) disk);
	disk = NULL;
    }
    return disk;
}

static int
DiskIO(
    DiskInfo	*disk,
    int		type,
    u_int	offset,
    u_int	count,
    void	*buffer)
{
    off_t	seekOffset;
    off_t	resultOffset;
    int		rc;
    ssize_t	amount;

    if (disk == NULL) {
	rc = 1;
	errno = EINVAL;
	goto done;
    }
    if (offset >= disk->sectors) {
        rc = 1;
        errno = EINVAL;
        goto done;
    }
    seekOffset = offset * DISK_SECTOR_SIZE;
    resultOffset = lseek(disk->fd, seekOffset, SEEK_SET);
    if (resultOffset != seekOffset) {
	rc = 1;
	goto done;
    }
    switch (type) {
	case DISK_READ: 
	    amount = read(disk->fd, buffer, count * DISK_SECTOR_SIZE);
	    break;
	case DISK_WRITE:
	    amount = write(disk->fd, buffer, count * DISK_SECTOR_SIZE);
	    break;
	default:
	    fprintf(stderr, "Internal error in DiskIO\n");
	    rc = 1;
	    goto done;
    }
    if (amount != (count * DISK_SECTOR_SIZE)) {
	rc = 1;
	if (errno == 0) {
	    errno = EIO;
	}
	goto done;
    }
    if ((disk->flags & DISK_ASYNC) == 0) {
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = 10000000;
	rc = nanosleep(&req, NULL);
	if (rc) {
	    rc = 1;
	    goto done;
	}
    }
    rc = 0;
done:
    return rc;
}

int
Disk_Read(
    Disk	diskHandle,
    u_int	offset,
    u_int	count,
    void	*buffer)
{
    DiskInfo	*disk = (DiskInfo *) diskHandle;
    int		rc;

    rc = DiskIO(disk, DISK_READ, offset, count, buffer);
    if (rc == 0) {
	disk->readOps++;
	disk->readSectors += count;
    }
    return rc;
}

int
Disk_Write(
    Disk	diskHandle,
    u_int	offset,
    u_int	count,
    void	*buffer)
{
    DiskInfo	*disk = (DiskInfo *) diskHandle;
    int		rc;

    rc = DiskIO(disk, DISK_WRITE, offset, count, buffer);
    if (rc == 0) {
	disk->writeOps++;
	disk->writeSectors += count;
    }
    return rc;
}


int
Disk_Close(
    Disk	diskHandle)
{
    DiskInfo	*disk = (DiskInfo *) diskHandle;
    int		rc;

    if (disk == NULL) {
	rc = 1;
	errno = EINVAL;
	goto done;
    }
    rc = close(disk->fd);
    if (rc) {
	rc = 1;
	goto done;
    }
    if ((disk->flags & DISK_SILENT) == 0) {
	fprintf(stderr, "Disk read ops: %lld\n", disk->readOps);
	fprintf(stderr, "Disk read sectors: %lld\n", disk->readSectors);
	fprintf(stderr, "Disk write ops: %lld\n", disk->writeOps);
	fprintf(stderr, "Disk write sectors: %lld\n", disk->writeSectors);
    }
    free((void *) disk);
    rc = 0;
done:
    return rc;
}







