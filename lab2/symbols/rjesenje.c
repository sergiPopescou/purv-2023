#include <stdlib.h>
#include <elf.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


char ELF_SYMBOL_VISIBILITY[7][15] = {"DEFAULT", "INTERNAL", "HIDDEN", "PROTECTED",
                              "EXPORTED", "SINGLETON", "ELIMINATE"};

char ELF_INFO_TYPE[12][16] = {"NOTYPE", "OBJECT", "FUNC", "SECTION", "FILE", "COMMON", "TLS", "LOOS",
                        "HIOS", "LOPROC", "SPARC_REGISTER", "HIPROC"};

char ELF_INFO_BIND[7][10] = {"LOCAL", "GLOBAL", "WEAK", "LOOS", "HIOS", "LOPROC", "HIPROC"};

char* section_indexes_mapping(char **section_index, uint16_t value){
  switch(value){
    case 0:
      *section_index = "UNDEF";
      break;
    case 0xff00:
      *section_index = "LORESERVE";
      break;
    case 0xff01:
      *section_index = "AFTER";
      break;
    case 0xff1f:
      *section_index = "HIPROC";
      break;
    case 0xff20:
      *section_index = "LOOS";
      break;
    case 0xff3f:
      *section_index = "HIOS";
      break;
    case 0xfff1:
      *section_index = "ABS";
      break;
    case 0xfff2:
      *section_index = "COMMON";
      break;
    case 0xffff:
      *section_index = "XINDEX";
      break;
    default:
      sprintf(*section_index, "%d", value);
      break;
  }
}


/* funkcija koja vrsi formatirani ispis na konzolu */
// prima 3 argumenta
  // 1. argument je pokazivac na Elf64_Sym strukturu koja cuva podatke o tabeli simbola
  // 2. argument je broj ulaza u tabelu simbola
  // 3. argument je niz stringova u kojima se cuvaju imena simbola, dohvacena iz odgovarajuce string tabele
void print_symbols(Elf64_Sym *symbol_table, int num_of_entries, char *names){

  printf("%6s %-16s%5s %-14s%-7s%-10s%-9s %-30s\n", "Num:", "Value", "Size", "Type", "Bind", "Vis", "Ndx", "Name");
  for(int i = 0; i < num_of_entries; i++){
    printf("%5d:", i);
    printf(" %016lx", symbol_table[i].st_value);
    printf("%5lu ", symbol_table[i].st_size);
    printf("%-14s", ELF_INFO_TYPE[ELF64_ST_TYPE(symbol_table[i].st_info)]);
    printf("%-7s", ELF_INFO_BIND[ELF64_ST_BIND(symbol_table[i].st_info)]);
    printf("%-10s", ELF_SYMBOL_VISIBILITY[symbol_table[i].st_other]);
    char *section_index = (char *)calloc(1, sizeof(char));
    section_indexes_mapping(&section_index, symbol_table[i].st_shndx);
    printf("%-9s", section_index);
    printf(" %-30s", names + symbol_table[i].st_name);
    printf("\n");
  }
  printf("\n");
}



int main(int argc, char* argv[]){


  /* za ekstrakciju podaatka iz fajla koriste se strukture definisane u elf.h zaglavlju */
  Elf64_Ehdr *elf_header;
  Elf64_Shdr *section_header;
  Elf64_Sym *sym_tab;
  Elf64_Sym *dyn_sym;

  if(argc != 2){
    printf("Nije proslijedjen fajl kao argument komandne linije!\n");
    return -1;
  }

  const char* putanja = argv[1];
  int fd = open(putanja, O_RDONLY);

  if(fd < 0){
    printf("Greska prilikom otvaranja fajla\n");
    return -1;
  }

  struct stat sb;
  if(fstat(fd, &sb) == -1){
    printf("Greska sa fajlom\n");
  }


  /* ucitavanja sadrzaja fajla u program odnosno u radnu memoriju, koristenjem mmap funkcije */
  char *elf_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

  if(elf_in_memory == MAP_FAILED){
    printf("Mapiranje nije uspjelo\n");
    return -1;
  }

  /* prvo cemo dohvatiti header elf fajla i iz njega procitati neke korisne vrijednosti */
  elf_header = (Elf64_Ehdr *) elf_in_memory;
  
  /* section_header je niz zaglavlja sekcija ELF fajla, pri cemu je jedna od sekcija tabela simbola */
  section_header = (Elf64_Shdr *)((char *) elf_in_memory + elf_header->e_shoff);  // polje e_shoff daje offset na kojem se nalazi pocetak section header-a



  
  int sym_num = -1, dynsym_num = 1;
  char *sym_names, *dynsym_names;

  /* dio koda koji prolazi kroz sve sekcije i na osnovu polja koje daje tip (sh_type) pronalazi tabele simbola .symtab i .dynsym */
  for(int i = 0; i < elf_header->e_shnum; i++){

    if(section_header[i].sh_type == SHT_SYMTAB){
      sym_tab = (Elf64_Sym *)((char *) elf_in_memory + section_header[i].sh_offset);                // zaglavlje pronadjene sekcije daje nam podatke o samoj sekciji (koja je takodje niz unosa)
      sym_num = section_header[i].sh_size / section_header[i].sh_entsize;                          // sh_offset pocetak sekcije, sh_size velicinu, sh_entsize velicinu jednog unosa
      sym_names = (char *)(elf_in_memory + section_header[section_header[i].sh_link].sh_offset);   // sh_link nam daje indeks unosa u tabelu sekcija na kojem se nalaze podaci sa imenima tabele simbola
    }

    if(section_header[i].sh_type == SHT_DYNSYM){                                                   // slicno ili identicno je i za
      dyn_sym = (Elf64_Sym *)((char *) elf_in_memory + section_header[i].sh_offset);               // drugu tabelu, odnosno
      dynsym_num = section_header[i].sh_size / section_header[i].sh_entsize;                       // .dynsym 
      dynsym_names = (char *)(elf_in_memory + section_header[section_header[i].sh_link].sh_offset);
    }
  }

  printf("Symbol table '.dynsym' contains %d entries:\n", dynsym_num);
  print_symbols(dyn_sym, dynsym_num, dynsym_names);

  printf("Symbol table '.symtab' contains %d entries:\n", sym_num);
  print_symbols(sym_tab, sym_num, sym_names);

  munmap(elf_in_memory, sb.st_size);
  close(fd);

  return 0;
}
