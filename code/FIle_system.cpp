#include <iostream>
#include <cmath>
#include <fstream>
#include <string.h>

using namespace std;

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
      return (14 + this->sectors_to_bitmap) * this->bytes_per_sector;
    }

    int get_bitmap_position() {
      return 14 * this->bytes_per_sector; 
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

    int copy_disk_to_system(char insert_file_name[30]) {
      FILE *insert_file, *system_file;
      root_directoty_entrie aux_entrie;

      insert_file = fopen(insert_file_name, "r");

      fseek(insert_file, 0, SEEK_END); // vai para o fim do arquivo
      int file_size = ftell(insert_file); // retorna onde o ponteiro esta (fim do arquivo), ou seja o tamanho do arquivo que queremos inserir

      system_file = fopen(this->file_system_name, "r+"); 

      int sectors_needed_to_data = ceil(float(file_size) / float(bytes_per_sector));
      cout << sectors_needed_to_data << " Sectors needed\n";

      fseek(system_file, 512, SEEK_SET); // vai para o root dir
      int has_space_in_rd = 0, rd_new_file_position;

      for(int i = 0; i < 192; i++) { // verifica se ha espaço para inserção de mais um arquivo
        fread(&aux_entrie, 32, 1, system_file);

        if (aux_entrie.name[0] == 0 || aux_entrie.name[0] == 0xE5) {
          has_space_in_rd= 1;
          rd_new_file_position = i;
          break;
        }
      }
      if(has_space_in_rd == 0) {// se nao houver espaco no root dir return
        return 0; 
      }

      fseek(system_file, this->get_bitmap_position(), SEEK_SET); // vai para o bitmap
      int sectors_in_system_to_data = this->total_sectors_amount - 14 - this->sectors_to_bitmap;
      int contiguos_free_sectors = 0, has_space_in_bitmap = 0, free_sector_data_position, position_newfile_bitmap;
      char reader, verify = 0;

      int number_of_bits_to_verify;

      for(int i = 0; i < ceil(float(sectors_in_system_to_data)/8); i++) { // verifica se ha o espaco contiguo necessario para insercao
        fread(&reader, sizeof(reader), 1, system_file); // Le cada byte

        if ((i+1) == ceil(float(sectors_in_system_to_data)/8)) {
          number_of_bits_to_verify = sectors_in_system_to_data % 8;
        }
        else {
          number_of_bits_to_verify = 8;
        }

        for(int j = 0; j < number_of_bits_to_verify; j++) {
          verify = get_bit(reader, (j % 8)); // verifica os n bits de cada byte

          if(!verify) contiguos_free_sectors++;
          else contiguos_free_sectors = 0;

          if (contiguos_free_sectors == sectors_needed_to_data) { // Caso ja tenha o espaco necessario 
            has_space_in_bitmap = 1;
            position_newfile_bitmap = (i*8) + (j+1) - sectors_needed_to_data; // ARMAZENA QUAL POSICAO DO BITMAP VAI SER UTILIZADA PARA ALOCAR OS DADOS
            free_sector_data_position = (((i*8) + (j+1) - sectors_needed_to_data) * 512) + this->get_data_position(); // posicao absoluta de onde os dados devem começar ser escritos
            break; // Sai da verificacao 
          }
        }

        if (has_space_in_bitmap) {break;} // Sai da verificacao 
      }

      if(has_space_in_bitmap == 0) {
        return 0; // se nao houver espaco para os dados return
      }

      char teste[32];
      fseek(insert_file, 0, SEEK_SET); // posicao 0 do arquivo a ser inserido
      fseek(system_file, free_sector_data_position, SEEK_SET); // posicao absoluta de onde os dados começam 

      for(int i = 0; i <= (file_size/32); i++) { // para cada 32 bytes de dados a ser inseridos

        if(i == file_size/32) { // se estiver no fim do dado a ser inserido e ele não ocupar exatamente 32 bytes
          fread(&teste, file_size%32 , 1, insert_file); // le e escreve apenas os bytes restantes
          fwrite(teste, file_size%32, 1, system_file);
        }
        else {
          fread(&teste, sizeof(teste), 1, insert_file);
          fwrite(teste, sizeof(teste), 1, system_file);
        }
      }
      int first_time = 1;

      for(int i = 0; i <= (sectors_needed_to_data / 8); i++) {
        fseek(system_file, this->get_bitmap_position() + (position_newfile_bitmap/512) + i, SEEK_SET); // vai na posicao do BYTE onde o bitmap deve ser marcado
        fread(&reader, sizeof(reader), 1, system_file);
        fseek(system_file, this->get_bitmap_position() + (position_newfile_bitmap/512) + i, SEEK_SET);
        // cout << "i: " << i << "\n"; 
        // cout << "S: " << sectors_needed_to_data/8 << "\n\n";

        if(i > 0){
          first_time = 0;
        }

        if((!((i) == sectors_needed_to_data / 8) && !first_time)){ // se nao estiver nos ultimos setores e nao for a primeira vez 
          for(int bit = 0; bit < 8; bit++){ // seta apenas todos os bits
              reader ^= (1 << bit); 
          }
        }
        else if((!((i) == sectors_needed_to_data / 8) && first_time)){ // se nao estiver nos ultimos setores e for a primeira vez 
          for(int bit = position_newfile_bitmap % 8; bit < 8; bit++){ // seta os bits partindo de uma posicao possivelmente no meio do byte 
              reader ^= (1 << bit);
          }
        }
        else if(((i) == sectors_needed_to_data / 8 && !first_time)){ // se estiver nos ultimos setores e nao for a primeira vez 
          for(int bit = 0; bit < sectors_needed_to_data % 8; bit++){ // seta os bits do comeco do byte ate possivelmente no meio
              reader ^= (1 << bit); // o bit da posicao (position_newfile_bitmap % 8 + bit) é setado para 1
          }
        }
        else if ((i) == sectors_needed_to_data / 8 && first_time){ // se estiver nos ultimos setores e for a primeira vez 
            for(int bit = position_newfile_bitmap % 8; bit < sectors_needed_to_data % 8; bit++){ // seta partindo possivelmente do meio ate possivelmente no meio
              reader ^= (1 << bit); 
          }
        }
        fwrite(&reader, sizeof(reader), 1, system_file); // reescreve o byte com a alteração do bit
      }

      fseek(system_file, 512 + (rd_new_file_position*32), SEEK_SET); // insere o nome no root dir 
      fwrite(insert_file_name, strlen(insert_file_name)-4, 1, system_file); // strlen -4 para retirar o ".txt"

      fwrite(insert_file_name, strlen(insert_file_name)-4, 1, system_file);

    
      fclose(system_file);
      fclose(insert_file);

      return 1;
    }

    // int remove_file(char insert_file_name[30]) {
    //   FILE *system_file;
    //   root_directoty_entrie aux_entrie;

    //   system_file = fopen(this->file_system_name, "r+");



    // }
};



int main()
{
  char file_system_name[30] = "teste.img", file_name[30] = "teste.txt", file_name2[30] = "teste2.txt";

  FILE *file;
  
  int amount_of_sectors = 30, deu_certo;

  //cout << "Nome do Sistema de Arquivos: ";
  //scanf("%s", file_system_name);

  //cout << "Nome do Arquivo a ser inserido: ";
  //scanf("%s", file_name);

  File_system file_system (file_system_name, amount_of_sectors);

  deu_certo = file_system.copy_disk_to_system(file_name2);

  if (!deu_certo){
    cout << "Nao foi possivel inserir\n";
  }

  return 0;
}