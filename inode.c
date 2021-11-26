#include "inode.h"

const int32_t ID_ITEM_FREE = 0;

void fill_default_sb(struct superblock *sb) {
   // add disk_size param
   strcpy(sb->signature, "== ext ==");
   strcpy(sb->volume_descriptor, "popis bla bla bla");
   sb->disk_size = 800; // 800B
   sb->datablock_size = 32; // takže max. počet "souborových" položek na cluster jsou dvě, protože sizeof(directory_item) = 16B
   sb->datablock_count = 10;
   sb->inode_count = 10;
   sb->bitmapi_start_address = sizeof(struct superblock); // konec sb
   sb->bitmap_start_address = sb->bitmapi_start_address + 10; // 80 bitů bitmapa
   sb->inode_start_address = sb->bitmap_start_address + 10;
   sb->data_start_address = sb->inode_start_address + 10 * sizeof(struct pseudo_inode);
}

void fill_superblock(struct superblock *sb, int32_t size, int32_t datablock_size) {
   strcpy(sb->signature, "hintik");
   strcpy(sb->volume_descriptor, "Semestralni prace KIV/ZOS");
   sb->disk_size = size;

   sb->datablock_size = datablock_size;
   
   size = size - sizeof(struct superblock);

   double bytes_per_inode = datablock_size;

   int inodes = ceil(size / bytes_per_inode);
   sb->inode_count = inodes;

   size = size - inodes * sizeof(struct pseudo_inode);


   size = size - ceil(inodes / 8.0);

   sb->datablock_count = size / datablock_size; 

   size = size - ceil(sb->datablock_count / 8.0);

   size = size - sb->datablock_count * datablock_size;

   sb->bitmapi_start_address = sizeof(struct superblock);
   sb->bitmap_start_address = sb->bitmapi_start_address + ceil(inodes / 8.0);

   sb->inode_start_address = sb->bitmap_start_address + ceil(sb->datablock_count / 8.0);

   sb->data_start_address = sb->inode_start_address + inodes * sizeof(struct pseudo_inode);

   printf("Zbyle misto: %d\n", size);

}

int get_free_inode_id(file_system *fs) {
   u_char inode_bitmap[fs->sb->bitmap_start_address - fs->sb->bitmapi_start_address];
   memset(inode_bitmap, '\0', sizeof(inode_bitmap));
   fseek(fs->file, fs->sb->bitmapi_start_address, SEEK_SET);
   fread(inode_bitmap, sizeof(inode_bitmap), 1, fs->file);

   for (int i = 0 ; i < sizeof(inode_bitmap); i++) {
         for (int j=7; j>=0; j--) {
            if(((inode_bitmap[i] >> j) & 0b1) == 0b0) {
               fseek(fs->file, fs->sb->bitmapi_start_address + i * sizeof(u_char), SEEK_SET);
               inode_bitmap[i] |= 1UL << j;
               fwrite(&inode_bitmap[i], sizeof(u_char), 1, fs->file);
               return i*8+8-j;
            }
         }
   }

   return 0;
}

int get_free_datablock_id(file_system *fs) {

   u_char data_bitmap[fs->sb->inode_start_address - fs->sb->bitmap_start_address];
   memset(data_bitmap, '\0', sizeof(data_bitmap));
   fseek(fs->file, fs->sb->bitmap_start_address, SEEK_SET);
   fread(data_bitmap, sizeof(data_bitmap), 1, fs->file);

   for (int i = 0 ; i < sizeof(data_bitmap); i++) {
         for (int j=7; j>=0; j--) {
            if(((data_bitmap[i] >> j) & 0b1) == 0b0) {
               fseek(fs->file, fs->sb->bitmap_start_address + i * sizeof(u_char), SEEK_SET);
               data_bitmap[i] |= 1UL << j;
               fwrite(&data_bitmap[i], sizeof(u_char), 1, fs->file);
               return i*8+8-j;
            }
         }
   }

   return 0;

}

int create_inode(file_system *fs) {
   int id = get_free_inode_id(fs);

   if(id < 0) {
      return EXIT_FAILURE;
   }

   printf("Volný inode: %d\n", id);

   return EXIT_SUCCESS;
}

int create_root_directory(file_system *fs) {
   int i;
   int inode_id = get_free_inode_id(fs);
   int data_id = get_free_datablock_id(fs);

   printf("Data: %d\n", inode_id);

   if(inode_id == 0 || data_id == 0) {
      return EXIT_FAILURE;
   }

   struct pseudo_inode inode = {};
   (&inode)->isDirectory = true;
   (&inode)->nodeid = inode_id;
   (&inode)->references = 1;

   (&inode)->direct[0] = data_id;

   for(i = 1; i < DIRECT_LINKS_COUNT; i++) {
      (&inode)->direct[i] = 0;
   }

   (&inode)->indirect1 = 0;
   (&inode)->indirect2 = 0;

   set_file_inode_position(fs, inode_id);
   fwrite(&inode, sizeof(inode), 1, fs->file);

   set_file_datablock_position(fs, data_id);
   struct directory_item di_root = {inode_id, "."};
   fwrite(&di_root, sizeof(struct directory_item), 1, fs->file);
   struct directory_item di_up = {inode_id, ".."};
   fwrite(&di_up, sizeof(struct directory_item), 1, fs->file);

   return EXIT_SUCCESS;
}

int create_directory(file_system *fs, int32_t parent, char *name) {
   if(parent < 1) {
      printf("Neplatna cesta");
      return EXIT_FAILURE;
   }
   printf("Pokousim se vytvorit slozku s nazvem %s\n", name);
   struct pseudo_inode parent_inode;
   int i;
   int free_directory_item, free_directory_item_block;
   printf("tady 00\n");
   set_file_inode_position(fs, parent);
   fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);

   printf("%d\n", parent);
   printf("%d, %d, %d\n", (&parent_inode)->isDirectory, (&parent_inode)->nodeid, (&parent_inode)->direct[0]);
   printf("Nazev: %s\n", fs->file_name);

   bool isD = (&parent_inode)->isDirectory;

   if(!isD) {
      printf("Zadany soubor neni slozka\n");
      return EXIT_FAILURE;
   }

   for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
      if((&parent_inode)->direct[i] != 0) {
         if(find_directory_item_in_datablock(fs, (&parent_inode)->direct[i], name) != 0) {
            printf("Soubor s takovym nazvem jiz existuje.\n");
            return EXIT_FAILURE;
         }

         printf("V tomhle datablocku není! %d\n", i);
      }
   }

   for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
      if((free_directory_item = find_free_directory_item_in_datablock(
         fs, (&parent_inode)->direct[i])) != 0) {
            free_directory_item_block = i;
            break;
      }
      
   } 

   int32_t inode_id = get_free_inode_id(fs);
   int32_t data_id = get_free_datablock_id(fs);

   if(inode_id == 0 || data_id == 0) {
      printf("Nebylo mozne ulozit soubor.\n");
      return EXIT_FAILURE;
   }
   
   printf("tady 3\n");

   struct pseudo_inode inode = {};
   (&inode)->isDirectory = true;
   (&inode)->nodeid = inode_id;
   (&inode)->references = 1;
   (&inode)->direct[0] = data_id;
   (&inode)->direct[1] = 0;
   (&inode)->direct[2] = 0;
   (&inode)->direct[3] = 0;
   (&inode)->direct[4] = 0;
   (&inode)->indirect1 = 0;
   (&inode)->indirect2 = 0;


   set_file_inode_position(fs, inode_id);
   fwrite(&inode, sizeof(inode), 1, fs->file);
   

   set_file_datablock_position(fs, data_id);
   struct directory_item di_root = {inode_id, "."};
   fwrite(&di_root, sizeof(struct directory_item), 1, fs->file);
   struct directory_item di_up = {parent, ".."};
   fwrite(&di_up, sizeof(struct directory_item), 1, fs->file);

   struct directory_item item = {};
   (&item)->inode = inode_id;
   strcpy((&item)->item_name, name);
   printf("Název: %s\n", (&item)->item_name);
   
   set_file_datablock_position(fs, (&parent_inode)->direct[free_directory_item_block]);
   fseek(fs->file, free_directory_item * sizeof(struct directory_item), SEEK_CUR);
   fwrite(&item, sizeof(struct directory_item), 1, fs->file);

   return EXIT_SUCCESS;
}

int find_free_directory_item_in_datablock(file_system *fs, int32_t datablock) {
   int i;
   int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
   struct directory_item items[item_count];

   set_file_datablock_position(fs, datablock);
   fread(items, sizeof(items), 1, fs->file);
   for(i = 0; i < item_count; i++) {
      if((&items[i])->inode == 0) {
         return i;
      }
   }

   return 0;
}

int find_directory_item_in_datablock(file_system *fs, int32_t datablock, char *name) {
   int i;
   int item_count = fs->sb->datablock_size/sizeof(struct directory_item);
   struct directory_item items[item_count];

   set_file_datablock_position(fs, datablock);
   fread(items, sizeof(items), 1, fs->file);
   for(i = 0; i < item_count; i++) {
      if(strcmp((&items[i])->item_name, name) == 0) {
         printf("Nalezen!\n");         
         return (&items[i])->inode;
      }
   }

   return 0;

}

int create_datablock(file_system *fs) {
   int id = get_free_datablock_id(fs);

   if(id < 0) {
      return EXIT_FAILURE;
   }

   printf("Volný blok: %d\n", id);
   return EXIT_SUCCESS;
}

int set_file_inode_position(file_system *fs, int32_t inode_id) {
   printf("# SB| ISA: %d | sizeof(inode): %d\n", fs->sb->inode_start_address, sizeof(struct pseudo_inode));
   printf("Skáču na inode (%d) pozici: %d\n", inode_id, fs->sb->inode_start_address + (inode_id - 1) * sizeof(struct pseudo_inode));
   return fseek(fs->file, fs->sb->inode_start_address + (inode_id - 1) * sizeof(struct pseudo_inode), SEEK_SET);
}

int set_file_datablock_position(file_system *fs, int32_t datablock_id) {
   printf("Skáču na data pozici: %d\n", fs->sb->data_start_address + (datablock_id - 1) * fs->sb->datablock_size);
   return fseek(fs->file, fs->sb->data_start_address + (datablock_id - 1) * fs->sb->datablock_size, SEEK_SET);
}

int32_t get_directory_item_inode(file_system *fs, int32_t parent, char* name) {
   
   struct pseudo_inode parent_inode;
   int i;
   int32_t inode_number;

   set_file_inode_position(fs, parent);
   fread(&parent_inode, sizeof(struct pseudo_inode), 1, fs->file);

   for(i = 0; i < DIRECT_LINKS_COUNT; i++) {
      inode_number = find_directory_item_in_datablock(fs, (&parent_inode)->direct[i], name);
      printf("Nalezeno v datablocku: %d inode %d\n", i, inode_number);

      if(inode_number != 0) {
         return inode_number;
      }
   }

   return 0;

}

int32_t get_inode_by_path(file_system *fs, int32_t parent, char *path) {
   printf("Počítám počet prochazejicich inodu.\n");

   if(strlen(path) == 0) {
      return parent;
   }

   int length = 0;
   int files_count = 1;

   while(path[length] != '\0') {
      if(path[length] == '/') files_count++;
      length++;
   }

   if(path[length - 1] == '/' && length > 1) {
      files_count--;
   }
   
   if(path[0] == '/') {
      parent = 1;
      files_count--;
   }

   printf("\nProjdu přes %d inodů\n", files_count);

   char *folder = strtok(path, "/");

   printf("Projdu (%d) přes složky: \n", parent);
   while(folder != NULL) {
      parent = get_directory_item_inode(fs, parent, folder);
      printf("%s - %d\n", folder, parent);
      folder = strtok(NULL, "/");
   }
   printf("Toť vše. %d, %s \n", parent, path);
   return parent;
}

int free_inode(file_system *fs, int32_t inode_id) {

   return EXIT_SUCCESS;

}

int free_datablock(file_system *fs, int32_t datablock_id) {

   return EXIT_SUCCESS;
}