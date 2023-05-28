#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Nije proslije]en fajl kao agrument komandne linije!\n");
        return -1;
    }
	
    // Open the ELF file
    FILE *fp;
    Elf64_Ehdr elf_header;
    Elf64_Shdr section_header;
    int i, j;

    fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("Error opening file\n");
        return -1;
    }
    // Read the ELF header
    fread(&elf_header, sizeof(Elf64_Ehdr), 1, fp);
    //Read the section header
    fseek(fp, elf_header.e_shoff + elf_header.e_shstrndx * sizeof (section_header), SEEK_SET);
    fread(&section_header, 1, sizeof section_header, fp);
    //Section name definition
    char * section_name = malloc(section_header.sh_size);
    fseek(fp, section_header.sh_offset, SEEK_SET);
    fread(section_name, 1, section_header.sh_size, fp);
    // Print section details
    printf("There are %d section headers, starting at offset 0x%lx:\n\n",
           elf_header.e_shnum, elf_header.e_shoff);

    printf("Section Headers:\n");
    printf("  [Nr] Name  \n");            
    printf("       Type              Address          Offset            Link\n");
    printf("       Size              EntSize          Info              Align\n");
    printf("       Flags\n");
      
    for (i = 0; i < elf_header.e_shnum; i++) {
        fseek(fp, elf_header.e_shoff + i* elf_header.e_shentsize, SEEK_SET);
        fread(&section_header, 1, sizeof(section_header), fp);

        printf("  [%2d] ", i);
	
	const char* name = "";
	if(section_header.sh_name){
	name = section_name + section_header.sh_name;
        printf("%-17s \n", name);
}
 
        // Print the section type
        switch (section_header.sh_type) {
            case SHT_NULL:
                printf("\n       NULL             ");
                break;
            case SHT_PROGBITS:
                printf("       PROGBITS         ");
                break;
            case SHT_SYMTAB:
                printf("       SYMTAB           ");
                break;
            case SHT_STRTAB:
                printf("       STRTAB           ");
                break;
            case SHT_RELA:
                printf("       RELA             ");
                break;
            case SHT_HASH:
                printf("       HASH             ");
                break;
            case SHT_DYNAMIC:
                printf("       DYNAMIC          ");
                break;
            case SHT_NOTE:
                printf("       NOTE             ");
                break;
            case SHT_NOBITS:
                printf("       NOBITS           ");
                break;
            case SHT_REL:
                printf("       REL              ");
                break;
            case SHT_SHLIB:
                printf("       SHLIB            ");
                break;
            case SHT_DYNSYM:
                printf("       DYNSYM           ");
                break;
            default:
                printf("       %016x ", section_header.sh_type);
                break;
        }

        printf("%016lx ", section_header.sh_addr);
        printf("%016lx ", section_header.sh_offset);
        printf("%5d \n", section_header.sh_link);
        printf("       %016lx ", section_header.sh_size);
        printf("%016lx ", section_header.sh_entsize);
        printf("%-20d ", section_header.sh_info);
        
        printf("%ld\n", section_header.sh_addralign);
        printf("       [%016lx]: ", section_header.sh_flags);
        
        //separate flags
        int k;
        int *bits = malloc(sizeof(int)* 16);
        for(k=0; k<16; k++){
        	int mask = 1 << k;
        	int masked_flags = (int)section_header.sh_flags & mask;
        	int thebit = masked_flags >> k;
        	bits[k]= thebit;
        }
        char *flags[13] = { "WRITE", "ALLOC", "EXEC", " ", "MERGE", "STRINGS", "INFO LINK", "LINK ORDER", "OS NONCONFORMING", "GROUP", "TLS", "MASKOS", "MASKPROC" };
        //print flags
        for(k=0; k<16; k++) {
        	if(bits[k]) printf ("%s ", flags[k]);    	
        }
        if(bits[0]==bits[2]==bits[3]==bits[4]==bits[5]==bits[6]==bits[7]==bits[8]==bits[9]==bits[10]==bits[11]==bits[12]==bits[13]==bits[14]==bits[15]==bits[16]==0) printf (" ");
        printf("\n");
}
           fclose(fp);

    return 0;
    }

