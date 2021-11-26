#include "filesystem.h"

file_system file_system_open(char *file_name) {
    
    file_system FS = { file_name, fopen(file_name, "wb+") };

    return FS;
}

int file_system_format(file_system *fs, int size) {
    int i;
    char *nula = '\0';

    printf("Formatuji\n");

    struct superblock sb = {};
    fill_default_sb(&sb);

    for(i = 0; i < size; i++) {
        fwrite(&nula, sizeof(nula), 1, fs->file);
    }

    fill_superblock(&sb, size, 4*1024);

    printf("Size konkr.:%d\n", sizeof(sb));
    printf("Size obecny.:%d\n", sizeof(struct superblock));

    printf("Výsledky formátování: \n");
    printf("Počet inodů: %d\n", (&sb)->inode_count);
    printf("Počet dat.blocků: %d\n", (&sb)->datablock_count);

    fs->sb = &sb;
    write_superblock(fs, &sb);
    

    int inode_bitmap_size = (&sb)->bitmap_start_address - (&sb)->bitmapi_start_address;
    int data_bitmap_size = (&sb)->inode_start_address - (&sb)->bitmap_start_address;

    u_char inode_bitmap[inode_bitmap_size];
    u_char data_bitmap[data_bitmap_size];

    memset(inode_bitmap, '\0', inode_bitmap_size);
    memset(data_bitmap, '\0', data_bitmap_size);

    printf("Velikost: %d\n", sizeof(inode_bitmap));

    printf("zapisuji bitmapy\n");
    fwrite(&inode_bitmap, sizeof(u_char), inode_bitmap_size, fs->file);
    fwrite(&data_bitmap, sizeof(u_char), data_bitmap_size, fs->file);
    //u_char inode_bitmap[inode_bitmap_size] = { 0 };
    //u_char data_bitmap[data_bitmap_size] = { 0 };
    printf("konec zapisovani bitmap\n");

    printf("halo?\n");
    create_root_directory(fs);

    printf("Tak tady?\n");
    create_directory(fs, 1, "homes");
    create_directory(fs, 1, "etc");
    create_directory(fs, 1, "var");
    create_directory(fs, 2, "hintik");
    create_directory(fs, 2, "petr");
    create_directory(fs, 6, "videa");
    create_directory(fs, 2, "hintik");


    //get_inode_by_path(fs, 1, "/file/obrazky/dovolena/");
    //get_inode_by_path(fs, 1, "file/obrazky/dovolena/");
    //get_inode_by_path(fs, 1, "/file/obrazky/dovolena");
    //get_inode_by_path(fs, 1, "file/obrazky/dovolena");

    return EXIT_SUCCESS;
}

int file_system_create(FILE file, int size, int block_size) {

    return EXIT_SUCCESS;
}


int write_superblock(file_system *fs, struct superblock* sb) {
    
    fseek(fs->file, 0, SEEK_SET);

    printf("Size:%d\n", sizeof(*sb));
    fwrite(&sb, sizeof(*sb), 1, fs->file);

    return EXIT_SUCCESS;
}

int file_system_close(file_system *fs) {
    fclose(fs->file);
    return EXIT_SUCCESS;
}