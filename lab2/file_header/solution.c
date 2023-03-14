#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

/*
    Program koji izvrsava istu radnju kao i Linux komanda readelf --file_header

    Rjesenje je bazirano na koristenju strukture Elf64_Ehdr koja predstavlja tip Elf zaglavlja
    U varijablu koja je ovog tipa, mozemo iz fajla ucitati zaglavlje, i lako pristupiti svim promjenljivima koje su definisane 
    u okviru date strukture.
    Na ovaj nacin lako dolazimo do podataka koji se ispisuju pri izvrsavanju date komande.

    Izvor od koristi: https://codebrowser.dev/linux/include/elf.h.html#65
*/

char ELF_CLASSES[3][20] = {"Invalid class", "ELF32", "ELF64"};
char ELF_DATA[3][50] = {"Invalid data encoding", "2's complement, little endian", "2's complement, big endian"};
char ELF_TYPE[5][30] = {"No file type", "Relocatable file", "Executable file", "Shared object file", "Core file"};

int OS_ABI_KEYS[14] = {0,1,2,3,6,7,8,9,10,11,12,64,97,255};
char OS_ABI_VALUES[14][50] = {"UNIX System V ABI", "HP-UX", "NetBSD.", "Object uses GNU ELF extensions.", "Sun Solaris.", "IBM AIX.","SGI Irix.", 
                    "FreeBSD.", "Compaq TRU64 UNIX.", "Novell Modesto.", "OpenBSD.", "ARM EABI", "ARM", "Standalone (embedded) application "};

int MACHINE_KEYS[82] = {0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 15, 17, 18, 19, 20, 21, 22, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 
                        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
                        80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
char MACHINE_VALUES[82][100] = {
    "No machine",
    "AT&T WE 32100",
    "SPARC",
    "Intel 80386",
    "Motorola 68000",
    "Motorola 88000",
    "Intel 80860",
    "MIPS I Architecture",
    "IBM System/370 Processor",
    "MIPS RS3000 Little-endian",
    "Hewlett-Packard PA-RISC",
    "Fujitsu VPP500",
    "Enhanced instruction set SPARC",
    "Intel 80960",
    "PowerPC",
    "64-bit PowerPC",
    "IBM System/390 Processor",
    "NEC V800",
    "Fujitsu FR20",
    "TRW RH-32",
    "Motorola RCE",
    "Advanced RISC Machines ARM",
    "Digital Alpha",
    "Hitachi SH",
    "SPARC Version 9",
    "Siemens TriCore embedded processor",
    "Argonaut RISC Core, Argonaut Technologies Inc.",
    "Hitachi H8/300",
    "Hitachi H8/300H",
    "Hitachi H8S",
    "Hitachi H8/500",
    "Intel IA-64 processor architecture",
    "Stanford MIPS-X",
    "Motorola ColdFire",
    "Fujitsu MMA Multimedia Accelerator",
    "Siemens PCP",
    "Sony nCPU embedded RISC processor",
    "Denso NDR1 microprocessor",
    "Motorola Star*Core processor",
    "Toyota ME16 processor",
    "STMicroelectronics ST100 processor",
    "Advanced Logic Corp. TinyJ embedded processor family",
    "Sony DSP Processor",
    "AMD x86-64 architecture",
    "Digital Equipment Corp. PDP-10",
    "Digital Equipment Corp. PDP-11",
    "Siemens FX66 microcontroller",
    "STMicroelectronics ST9+ 8/16 bit microcontroller",
    "STMicroelectronics ST7 8-bit microcontroller",
    "Motorola MC68HC16 Microcontroller",
    "Motorola MC68HC11 Microcontroller",
    "Motorola MC68HC08 Microcontroller",
    "Motorola MC68HC05 Microcontroller",
    "Silicon Graphics SVx",
    "STMicroelectronics ST19 8-bit microcontroller",
    "Digital VAX",
    "Axis Communications 32-bit embedded processor",
    "Infineon Technologies 32-bit embedded processor",
    "Element 14 64-bit DSP Processor",
    "LSI Logic 16-bit DSP Processor",
    "Donald Knuth's educational 64-bit processor",
    "Harvard University machine-independent object files",
    "SiTera Prism",
    "Atmel AVR 8-bit microcontroller",
    "Fujitsu FR30",
    "Mitsubishi D10V",
    "Mitsubishi D30V",
    "NEC v850",
    "Mitsubishi M32R",
    "Matsushita MN10300",
    "Matsushita MN10200",
    "picoJava",
    "OpenRISC 32-bit embedded processor",
    "ARC Cores Tangent-A5",
    "Tensilica Xtensa Architecture",
    "Alphamosaic VideoCore processor",
    "Thompson Multimedia General Purpose Processor",
    "National Semiconductor 32000 series",
    "Tenor Network TPC processor",
    "Trebia SNP 1000 processor",
    "STMicroelectronics ST200 microcontroller",
};

char* get_os_abi_value(int key){
    for(int i=0;i<14;i++)
        if(key == OS_ABI_KEYS[i])
            return OS_ABI_VALUES[i];
    return "Invalid key";
}

char* get_machine_value(int key){
    for(int i=0;i<82;i++)
        if(key == MACHINE_KEYS[i])
            return MACHINE_VALUES[i];
    return "Invalid key";
}

int main(int argc, char *argv[]) {
    Elf64_Ehdr elf_header; // Struktura koja predstavlja tip Elf zaglavlja
    if(argc != 2){
        printf("Putanja do izvrsnog fajla mora biti navedena kao argument komandne linije !\n");
        return -1;
    }
    FILE* fp = fopen(argv[1], "r"); // Otvaramo izvrsni fajl, u read modu
    if (fp == NULL) { // Provjera u slucaju greske pri otvaranju
        printf("Greska pri otvaranju fajla !\n");
        return -1;
    }
    fread(&elf_header, sizeof(Elf64_Ehdr), 1, fp); // Ucitavamo zaglavlje izvrsnog fajla u promjenljivu Elf header tipa
    // Magic number - prvih 16 bajtova fajla koji predstavljaju jedinstveni identifikator tipa fajla
    printf("Magic number: ");
    for(int i=0;i<EI_NIDENT;i++)
        printf("%02x ", elf_header.e_ident[i]);
    printf("\n");
    // Class - Daje arhitekturu sistema za koji je fajl kompajliran
    printf("Class:       %s\n", ELF_CLASSES[elf_header.e_ident[EI_CLASS]]); 
    // Data - Daje nacin reprezentacije podataka i nacin njegovog smjestanja na sistemu (big-endian, little-endian...)
    printf("Data:        %s\n", ELF_DATA[elf_header.e_ident[EI_DATA]]);
    // Version - Verzija ELF formata
    printf("Version:     %d (current)\n", elf_header.e_ident[EI_VERSION]);
    // OS/ABI - Koristeni operativni sistem i ABI (Application Binary Interface)
    printf("OS/ABI:      %s\n", get_os_abi_value(elf_header.e_ident[EI_OSABI]));
    // Verzija ABI-ja koju izvrsni fajl koristi
    printf("ABI Version: %d\n", elf_header.e_ident[EI_ABIVERSION]);
    /*
        Polje e_ident unutar strukture sluzi da sacuva magic numbers, kao i dodatne informacije (data, version,...)
        Samo polje je niz od 16 bajtova, a da bismo znali do kojeg podatka doci, to jeste kako indeksirati,
        koristimo odgovarajuce makroe definisane u samom heder fajlu.
        (Pogledati dokumentaciju ili link sa vrha za dostupne makroe)
    */
    // Type - Tip fajla (izvrsni, dijeljeni...)
    printf("Type:        %s\n", ELF_TYPE[elf_header.e_type]);
    // Machine - Arhitektura racunarskog sistema na kome se fajl izvrsava
    printf("Machine:     %s\n", get_machine_value(elf_header.e_machine));
    // Version - Verzija objektnog fajla
    printf("Version:     0x%d\n", elf_header.e_version);
    // Entry point - Ulazna tacka programa, virtuelna adresa sa koje pocinje izvrsavanje programa
    printf("Entry point: 0x%lx\n", elf_header.e_entry);
    // Start of program headers - Broj bajtova (offset) nakon kojeg krecu zaglavlja programa
    printf("Start of program headers:   %ld (bytes into file)\n", elf_header.e_phoff);
    // Start of section headers - Broj bajtova (offset) nakon kojeg krecu zaglavlja sekcija
    printf("Start of section headers:   %ld (bytes into file)\n", elf_header.e_shoff);
    // Flags - Specificni flegovi vezani za fajl ciji se header pregleda
    printf("Flags:       0x%x\n", elf_header.e_flags);
    // Size of this header - Velicina elf zaglavlja u bajtovima
    printf("Size of this header:     %d (bytes)\n", elf_header.e_ehsize);
    // Size of program headers - Velicina ulazne tabele zaglavlja programa u bajtovima
    printf("Size of program headers:     %d (bytes)\n", elf_header.e_phentsize);
    // Number of program headers - Broj prethodnih zaglavlja
    printf("Number of program headers:      %d\n", elf_header.e_phnum);
    // Size of section headers - Velicina ulazne tabele sekcija programa u bajtovima
    printf("Size of section headers:     %d (bytes)\n", elf_header.e_shentsize);
    // Number of section headers - Broj prethodnih zaglavlja
    printf("Number of section headers:      %d\n", elf_header.e_shnum);
    // Section header string table index - Indeks ulazne tabele sekcija koja sadrzi tabele stringova sekcija zaglavlja
    printf("Section header string table index:  %d\n", elf_header.e_shstrndx);

    /*
        Dakle, vidimo da u datoj strukturi postoje sva polja neophodna za opisivanje sadrzaja zaglavlja.
    */
    fclose(fp); 
    return 0;
}