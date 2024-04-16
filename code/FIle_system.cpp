#include <iostream>
#include <cmath>
#include <fstream>

using namespace std;

struct bit_variable
{
    unsigned char b1 : 1;
    unsigned char b2 : 1;
    unsigned char b3 : 1;
    unsigned char b4 : 1;
    unsigned char b5 : 1;
    unsigned char b6 : 1;
    unsigned char b7 : 1;
    unsigned char b8 : 1;
};

struct root_directoty_entrie {
  char name[13];
  char extension[3];
  unsigned int size_in_bytes;
  unsigned short type;
  unsigned int first_sector;
  char amount_of_sectors[6];
};

char get_bit(char reader, int index) {
  return 1 & (reader >> index);
}

// void set_bit_1(char reader, int index) {
//   reader << 
// }

class File_system {
  private:
    int bytes_per_sector = 512, sectors_to_root_dir = 12, total_sectors_amount = 0;
    int sectors_to_bitmap;
    char file_system_name[30];

    void initialize_variables(char file_system_name[30], int total_sectors) {

      this->total_sectors_amount = total_sectors;
      this->sectors_to_bitmap =  ceil(float((this->total_sectors_amount - 14)) / float((this->bytes_per_sector * 8)));

      for(int i = 0; i < 30; i++) { // Copia o nome do arquivo
        this->file_system_name[i] = file_system_name[i];
      }
    }

    int get_total_sectors_amount(char file_system_name[30]) { // tamanho total do sistema em setores
      ifstream file;
      int system_total_sectors_amount;

      file.open(file_system_name, ifstream::in); // in = only_read

      file.seekg(0, ios::end);

      system_total_sectors_amount = file.tellg();
      system_total_sectors_amount = system_total_sectors_amount / this->bytes_per_sector; // Converte para setores

      file.close();
      return system_total_sectors_amount;
    }

    // Cria o arquivo do tamanho determinado pelo usuario
    void create_img(){
      FILE *create;
      create = fopen(this->file_system_name, "w");
      
      char aux = 0;
      int size_in_bytes = this->bytes_per_sector * this->total_sectors_amount; // size_in_bytes do sistema inteiro

      for(int i = 0; i < size_in_bytes; i++) {
        fwrite(&aux, 1, 1, create); // (pointer, size of each element, number of elements, file)
      }
      fclose(create);
    }

    // Gera o boot record no arquivo já criado
    void generate_boot_record() {
      FILE *file;
      int entrie_size_rd = 32;
      
      int amount_entries_root_dir = (this->bytes_per_sector * this->sectors_to_root_dir) / entrie_size_rd;

      file = fopen(this->file_system_name, "r+");

      fwrite(&this->bytes_per_sector, 4, 1, file);
      fwrite(&amount_entries_root_dir, 4, 1, file);
      fwrite(&sectors_to_bitmap, 4, 1, file);

      fclose(file);
    }

    int get_data_position() {
      return (1 + this->sectors_to_root_dir + this->sectors_to_bitmap) * this->bytes_per_sector;
    }

    int get_bitmap_position() {
      return this->sectors_to_bitmap * this->bytes_per_sector; 
    }

  public:
    File_system(char *file_system_name, int total_sector_amount) { // Constrututor para um novo sistema de arquivos (2 parametros)
      this->initialize_variables(file_system_name, total_sector_amount);
      this->create_img();
      this->generate_boot_record();
    }

     File_system(char *file_system_name) { // Construtor para um sistema ja existente (1 parametro)
      int total_sectors_amount = this->get_total_sectors_amount(file_system_name);
      this->initialize_variables(file_system_name, total_sectors_amount);
    }

    int copy_disk_to_system(char *insert_file_name) {
      FILE *insert_file, *system_file;
      root_directoty_entrie aux_entrie;
      bit_variable reader_bits;

      cout << insert_file_name << "\n";

      insert_file = fopen(insert_file_name, "r");

      fseek(insert_file, 0, SEEK_END); // vai para o fim do arquivo
      int file_size = ftell(insert_file); // retorna onde o ponteiro esta (fim do arquivo)
      cout << "Tamanho do arquivo: " << file_size << "\n";

      system_file = fopen(this->file_system_name, "r+");

      int sectors_needed_to_data = file_size / bytes_per_sector;

      fseek(system_file, 512, SEEK_SET);
      int has_space_in_rd = 0;

      for(int i = 0; i < 192; i++) {
        fread(&aux_entrie, sizeof(aux_entrie), 1, system_file);

        if (aux_entrie.name[0] == 0 || aux_entrie.name[0] == 0xE5) {
          has_space_in_rd= 1;
          break;
        }
      }

      if(has_space_in_rd == 0) {
        return 0;
      }

      fseek(system_file, this->get_bitmap_position(), SEEK_SET);
      int sectors_in_system_to_data = this->total_sectors_amount - 14 - this->sectors_to_bitmap;

      int contiguos_free_sectors = 0;
      char reader, verify = 0;

      for(int i = 0; i < (sectors_in_system_to_data * bytes_per_sector); i++) {
        fread(&reader, sizeof(reader), 1, system_file); // Le cada byte

        for(int j = 0; j < 8; j++) {
          verify = get_bit(reader, (j % 8)); // verifica os 8 bits de cada byte

          if(!verify) {
            contiguos_free_sectors++;
          }
          else {contiguos_free_sectors = 0;}
        }
      }
      
      fclose(system_file);
      fclose(insert_file);

      return 1;
    }
};

int main()
{
  char file_system_name[30], file_name[30];
  FILE *file;
  
  int amount_of_sectors = 20;

  cout << "Nome do Sistema de Arquivos: ";
  scanf("%s", file_system_name);

  File_system file_system (file_system_name, 20);

  //file_system.copy_disk_to_system(file_system_name);

  return 0;
}