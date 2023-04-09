#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

       
        
        
        char* get_section_header_flags_symbols(unsigned int flags) {
    static char symbols[12] = {0};  // String to hold symbols, initialized to all zeroes
    int i = 0;                      // Index into symbols array

    if (flags & SHF_WRITE) {
        symbols[i++] = 'W';
    }

    if (flags & SHF_ALLOC) {
        symbols[i++] = 'A';
    }

    if (flags & SHF_EXECINSTR) {
        symbols[i++] = 'X';
    }

    if (flags & SHF_MERGE) {
        symbols[i++] = 'M';
    }

    if (flags & SHF_STRINGS) {
        symbols[i++] = 'S';
    }

    if (flags & SHF_INFO_LINK) {
        symbols[i++] = 'I';
    }

    if (flags & SHF_LINK_ORDER) {
        symbols[i++] = 'L';
    }

    if (flags & SHF_OS_NONCONFORMING) {
        symbols[i++] = 'O';
    }

    if (flags & SHF_GROUP) {
        symbols[i++] = 'G';
    }

    if (flags & SHF_TLS) {
        symbols[i++] = 'T';
    }

    if (flags & SHF_COMPRESSED) {
        symbols[i++] = 'C';
    }

    return symbols;
}

       


void print_section_headers(Elf64_Ehdr *elf_header, Elf64_Shdr *section_headers, int num_sections, char *section_names) {
    printf("There are %d section headers, starting at offset 0x%lx:\n\n", num_sections, elf_header->e_shoff);

    printf("Section Headers:\n");
    printf("  [Nr] Name              Type            Address          Offset\n");

    if (elf_header->e_machine == EM_X86_64) {
        printf("       Size             EntSize         Flags  Link  Info  Align\n");
    } else {
        printf("       Size             Align\n");
    }
    
  
  
       

    for (int i = 0; i < num_sections; i++) {
        Elf64_Shdr *section_header = &section_headers[i];
       char *flags_p = get_section_header_flags_symbols(section_headers[i].sh_flags);
  
        printf("  [%2d] %-17s %-15s %016lx  %08lx\n",
            i,
            &section_names[section_header->sh_name],
            section_header->sh_type == SHT_NULL ? "NULL" :                // zaglavlje sekcije je neaktivno
            section_header->sh_type == SHT_PROGBITS ? "PROGBITS" :      //  definisano od strane programa
            section_header->sh_type == SHT_SYMTAB ? "SYMTAB" :        
            section_header->sh_type == SHT_STRTAB ? "STRTAB" :         // sadrzi tabelu nizova
            section_header->sh_type == SHT_RELA ? "RELA" :           // sadrzi unose premjestanja     
            section_header->sh_type == SHT_HASH ? "HASH" :          // sadrzi hash tabelu simbola
            section_header->sh_type == SHT_DYNAMIC ? "DYNAMIC" :    // sadrzi info za dimamicko povezivanje
            section_header->sh_type == SHT_NOTE ? "NOTE" :          // sadrzi info koje opisuju datoteku        
            section_header->sh_type == SHT_NOBITS ? "NOBITS" :
            section_header->sh_type == SHT_REL ? "REL" :
            section_header->sh_type == SHT_SHLIB ? "SHLIB" :
            section_header->sh_type == SHT_DYNSYM ? "DYNSYM" :
            section_header->sh_type == SHT_INIT_ARRAY ? "INIT_ARRAY" :
            section_header->sh_type == SHT_FINI_ARRAY ? "FINI_ARRAY" :
            section_header->sh_type == SHT_PREINIT_ARRAY ? "PREINIT_ARRAY" :
            section_header->sh_type == SHT_GROUP ? "GROUP" :
           section_header->sh_type == SHT_SYMTAB_SHNDX ? "SYMTAB SECTION INDICES" :
            "UNKNOWN",
            section_header->sh_addr,
            section_header->sh_offset);
            
            

        if (elf_header->e_machine == EM_X86_64) {
            printf("       %016lx  %06lx          %3s   %3u   %3u    %lu\n",
                section_header->sh_size,
                section_header->sh_entsize,
                flags_p,
                section_header->sh_link,
                section_header->sh_info,
                section_header->sh_addralign);
                
    }
         else {
            printf("       %06lx          %06lx\n",
                section_header->sh_size,
                section_header->sh_addralign);
        }
         
    }
    
    printf(" Key of flags: \n");
    printf("    W (write), A (alloc), X (execute), M (merge), S (strings), I (info), \n");
    printf("    L (link order), O (extra OS processing required), G (group), T (TLS), \n");
    printf("    C (compressed), x (unknown), o (OS specific), E (exclude) \n ");
    
}

  // kao drugi argument potrebno je navesti izvrsni fajl 
  
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <elf-file>\n", argv[0]);
        exit(1);
    }

    // Open the ELF file and read in the ELF header
    FILE *elf_file = fopen(argv[1], "rb");
    if (!elf_file) {
        printf("Error: unable to open file %s\n", argv[1]);
        exit(1);
    }
            // citanje Ehdr  headera
    Elf64_Ehdr elf_header;
    if (fread(&elf_header, sizeof(Elf64_Ehdr), 1, elf_file) != 1) {
        printf("Error: unable to read ELF header\n");
        exit(1);
    }

    // Read in the section header table
    Elf64_Shdr *section_headers = malloc(sizeof(Elf64_Shdr) * elf_header.e_shnum);
    if (!section_headers) {
        printf("Error: unable to allocate memory for section headers\n");
        exit(1);
    }

    if (fseek(elf_file, elf_header.e_shoff, SEEK_SET) < 0) {
        printf("Error: unable to seek to section header table\n");
        exit(1);
    }

    if (fread(section_headers, sizeof(Elf64_Shdr), elf_header.e_shnum, elf_file) != elf_header.e_shnum) {
        printf("Error: unable to read section headers\n");
        exit(1);
    }

    // Read in the section header string table
    // Citanje sekcije koja sadrzi string tabelu 
    char *section_names = NULL;
    Elf64_Shdr *section_header_string_table = &section_headers[elf_header.e_shstrndx];
    if (section_header_string_table->sh_type == SHT_STRTAB) {
        section_names = malloc(section_header_string_table->sh_size);
        if (!section_names) {
            printf("Error: unable to allocate memory for section header string table\n");
            exit(1);
        }

        if (fseek(elf_file, section_header_string_table->sh_offset, SEEK_SET) < 0) {
            printf("Error: unable to seek to section header string table\n");
            exit(1);
        }

        if (fread(section_names, section_header_string_table->sh_size, 1, elf_file) != 1) {
            printf("Error: unable to read section header string table\n");
            exit(1);
        }
        
        
        
    }

    // Print out information about each section header
    print_section_headers(&elf_header, section_headers, elf_header.e_shnum, section_names);
    
 

    // Clean up and exit
    fclose(elf_file);
    free(section_headers);
    free(section_names);

    return 0;
}
