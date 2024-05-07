#include <iostream>
#include <cmath>
#include <fstream>
#include <string.h>
#include <limits>

using namespace std;

struct root_directoty_entrie {
  char name[15];
  char extension[3];
  unsigned int size_in_bytes;
  unsigned short type;
  unsigned int first_sector;
  unsigned int amount_sectors;
};

char get_bit(char reader, int index) {
  return 1 & (reader >> index);
}

void limparBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
    void file_system_not_created(char *file_system_name, int total_sector_amount) { // Constrututor para um novo sistema de arquivos (2 parametros)
      this->initialize_variables(file_system_name, total_sector_amount);
      this->create_img();
      this->generate_boot_record();
    }

    void file_system_already_created(char *file_system_name) { // Construtor para um sistema ja existente (1 parametro)
      int total_sectors_amount = this->get_total_sectors_amount(file_system_name);
      this->initialize_variables(file_system_name, total_sectors_amount);
    }

    root_directoty_entrie get_rd_entrie(char file_name_entrie[30]) {
      FILE *system_file;
      root_directoty_entrie aux_entrie;

      char name_without_extension[30];

      for(int i = 0; i < strlen(file_name_entrie)-4; i++) {
        name_without_extension[i] = file_name_entrie[i];
        name_without_extension[i+1] = '\0';
      }

      system_file = fopen(this->file_system_name, "r");
      fseek(system_file, this->bytes_per_sector, SEEK_SET);

      for(int i = 0; i < 192; i++) { // acha o arquivo no root directory
        fread(&aux_entrie.name, sizeof(aux_entrie.name), 1, system_file);
        fread(&aux_entrie.extension, sizeof(aux_entrie.extension), 1, system_file);
        fread(&aux_entrie.size_in_bytes, sizeof(aux_entrie.size_in_bytes), 1, system_file);
        fread(&aux_entrie.type, sizeof(aux_entrie.type), 1, system_file);
        fread(&aux_entrie.first_sector, sizeof(aux_entrie.first_sector), 1, system_file);

        fseek(system_file, this->bytes_per_sector+i*32+28, SEEK_SET);
        fread(&aux_entrie.amount_sectors, 4, 1, system_file);

        if (!strcmp(name_without_extension, aux_entrie.name)) {
          return aux_entrie;
        }
      }

      cout << "Arquivo nao encontrado\n";
      return aux_entrie;
      fclose(system_file);
    }

    int copy_system_to_disk(char file_name_on_system[30]) {
      FILE *system_file, *new_file_in_disk;
      root_directoty_entrie aux_entrie;

      system_file = fopen(this->file_system_name, "rb");
      new_file_in_disk = fopen(file_name_on_system, "w+b");

      aux_entrie = this->get_rd_entrie(file_name_on_system);

      fseek(system_file, this->get_data_position() + aux_entrie.first_sector * this->bytes_per_sector, SEEK_SET);
      
      int resto = 512;
      char reader_arq[512];
      reader_arq[512] = '\0';

      for(int i = 0; i < ceil(float(aux_entrie.size_in_bytes) / 512); i++) {
        
        if(i+1 == ceil(float(aux_entrie.size_in_bytes))) {
          resto = aux_entrie.size_in_bytes % 512;
          fread(reader_arq, resto, 1, system_file);
          reader_arq[resto+1] = '\0';

          fwrite(reader_arq, resto, 1, new_file_in_disk);
        }

        else {
          fread(reader_arq, resto, 1, system_file);
          fwrite(reader_arq, resto, 1, new_file_in_disk);
        }
      }
      fclose(new_file_in_disk);
      fclose(system_file);

      return 0;
    }


    void list_files(){
      bool is_zero = false;
        FILE *system_file;
        root_directoty_entrie aux_entrie;
        int pos_rootdir = 512;
        
        system_file = fopen(file_system_name, "r");
      
      fseek(system_file, 512, SEEK_SET);

      while(!is_zero){

        fread(&aux_entrie.name, sizeof(aux_entrie.name), 1, system_file);
        fread(&aux_entrie.extension, sizeof(aux_entrie.extension), 1, system_file);
        fread(&aux_entrie.size_in_bytes, sizeof(aux_entrie.size_in_bytes), 1, system_file);
        fread(&aux_entrie.type, sizeof(aux_entrie.type), 1, system_file);
        fread(&aux_entrie.first_sector, sizeof(aux_entrie.first_sector), 1, system_file);

        if(aux_entrie.name[0] == 0){
          is_zero = true;
        }else if(aux_entrie.name[0] == -27){
          pos_rootdir += 32;
          fseek(system_file, pos_rootdir, SEEK_SET);
        } else{
          printf("Nome: ");
          for(int i=0; i<15; i++){
            printf("%c", aux_entrie.name[i]);
          }
          printf(".");
          for(int i=0; i<3; i++){
            printf("%c", aux_entrie.extension[i]);
          }
          printf("\nTamanho: %d\n", aux_entrie.size_in_bytes);
          pos_rootdir += 32;
          fseek(system_file, pos_rootdir, SEEK_SET);
          printf("\n");
        }
      }
      
      return;
    }

    int copy_disk_to_system(char insert_file_name[30]) {
      FILE *insert_file, *system_file;
      root_directoty_entrie aux_entrie;

      insert_file = fopen(insert_file_name, "rb");

      fseek(insert_file, 0, SEEK_END); // vai para o fim do arquivo
      int file_size = ftell(insert_file); // retorna onde o ponteiro esta (fim do arquivo), ou seja o tamanho do arquivo que queremos inserir

      system_file = fopen(this->file_system_name, "r+b"); 

      int sectors_needed_to_data = ceil(float(file_size) / float(bytes_per_sector));
      cout << sectors_needed_to_data << " Sectors needed   File size: " << file_size << "\n";

      fseek(system_file, 512, SEEK_SET); // vai para o root dir
      int has_space_in_rd = 0, rd_new_file_position;

      for(int i = 0; i < 192; i++) { // verifica se ha espaço para inserção de mais um arquivo
        fread(&aux_entrie, 32, 1, system_file);

        if (aux_entrie.name[0] == 0 || aux_entrie.name[0] == -27) {
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

      fseek(insert_file, 0, SEEK_SET); // posicao 0 do arquivo a ser inserido
      fseek(system_file, free_sector_data_position, SEEK_SET); // posicao absoluta de onde os dados começam

      char teste[32];
      teste[32] = '\0';

      for(int i = 0; i < ceil(float(file_size)/32); i++) { // para cada 32 bytes de dados a ser inseridos
        //for(int zera = 0; zera < 32; zera++){teste[zera] = 0;}

        if(i+1 == ceil(float(file_size)/32)) { // se estiver no fim do dado a ser inserido e ele não ocupar exatamente 32 bytes
          fseek(insert_file, i*32, SEEK_SET);
          fread(&teste, file_size%32, 1, insert_file); // le e escreve apenas os bytes restantes
          fwrite(teste, file_size%32, 1, system_file);
          break;
        }
        else {
          fread(&teste, 32, 1, insert_file);
          fwrite(teste, 32, 1, system_file);
        }
      }
      int first_time = 1;

      for(int i = 0; i < ceil(float(sectors_needed_to_data) / 8); i++) {
        fseek(system_file, this->get_bitmap_position() + (position_newfile_bitmap/8) + i, SEEK_SET); // vai na posicao do BYTE onde o bitmap deve ser marcado
        fread(&reader, sizeof(reader), 1, system_file);
        fseek(system_file, this->get_bitmap_position() + (position_newfile_bitmap/8) + i, SEEK_SET); // 512 é o problema

        if(i > 0){
          first_time = 0;
        }

        if((!((i) == (sectors_needed_to_data+position_newfile_bitmap) / 8) && !first_time)){ // se nao estiver nos ultimos setores e nao for a primeira vez 
          for(int bit = 0; bit < 8; bit++){ // seta apenas todos os bits
              reader ^= (1 << bit); 
          }
        }
        else if((!((i) == (sectors_needed_to_data+position_newfile_bitmap) / 8) && first_time)){ // se nao estiver nos ultimos setores e for a primeira vez 
          for(int bit = position_newfile_bitmap % 8; bit < 8; bit++){ // seta os bits partindo de uma posicao possivelmente no meio do byte 
              reader ^= (1 << bit);
          }
        }
        else if(((i) == (sectors_needed_to_data+position_newfile_bitmap) / 8 && !first_time)){ // se estiver nos ultimos setores e nao for a primeira vez 
          for(int bit = 0; bit < (sectors_needed_to_data+position_newfile_bitmap) % 8; bit++){ // seta os bits do comeco do byte ate possivelmente no meio
              reader ^= (1 << bit); // o bit da posicao (position_newfile_bitmap % 8 + bit) é setado para 1
          }
        }
        else if ((i) == (sectors_needed_to_data+position_newfile_bitmap) / 8 && first_time){ // se estiver nos ultimos setores e for a primeira vez 
            for(int bit = position_newfile_bitmap % 8; bit < (sectors_needed_to_data+position_newfile_bitmap) % 8; bit++){ // seta partindo possivelmente do meio ate possivelmente no meio
              reader ^= (1 << bit);
          }
        }
        fwrite(&reader, sizeof(reader), 1, system_file); // reescreve o byte com a alteração do bit
      }

      fseek(system_file, 512 + (rd_new_file_position*32), SEEK_SET); // insere o nome no root dir 
      fwrite(insert_file_name, strlen(insert_file_name)-4, 1, system_file); // strlen -4 para retirar o ".txt"

      char copy_string[4];
  
      for(int i = 0; i <= strlen(insert_file_name); i++) {
        if(i == strlen(insert_file_name)-3) {
          for(int j = 0; j < 3; j++) {
            copy_string[j] = insert_file_name[i+j];
          }
        }
      }
      copy_string[3] = '\0';

      fseek(system_file, 512 + (rd_new_file_position*32) + 15, SEEK_SET);
      fwrite(copy_string, strlen(copy_string), 1, system_file);

      fseek(system_file, 512 + (rd_new_file_position*32) + 18, SEEK_SET);

      fwrite(&file_size, sizeof(file_size), 1, system_file);

      unsigned short insert_file_type = 0x10;

      fseek(system_file, 512 + (rd_new_file_position*32) + 22, SEEK_SET);
      fwrite(&insert_file_type, sizeof(insert_file_type), 1, system_file);

      fseek(system_file, 512 + (rd_new_file_position*32) + 24, SEEK_SET);
      fwrite(&position_newfile_bitmap, sizeof(position_newfile_bitmap), 1, system_file);

      fseek(system_file, 512 + (rd_new_file_position*32) + 28, SEEK_SET);
      fwrite(&sectors_needed_to_data, sizeof(sectors_needed_to_data), 1, system_file);

      fclose(system_file);
      fclose(insert_file);

      return 1;
    }

    int remove_file(char remove_file_name[30]) {
      FILE *system_file;
      root_directoty_entrie aux_entrie;

      char name_without_extension[30];

      for(int i = 0; i < strlen(remove_file_name)-4; i++) {
        name_without_extension[i] = remove_file_name[i];
        name_without_extension[i+1] = '\0';
      }

      system_file = fopen(this->file_system_name, "r+");
      fseek(system_file, this->bytes_per_sector, SEEK_SET);

      for(int i = 0; i < 192; i++) { // acha o arquivo no root directory
        fread(&aux_entrie.name, sizeof(aux_entrie.name), 1, system_file);
        fread(&aux_entrie.extension, sizeof(aux_entrie.extension), 1, system_file);
        fread(&aux_entrie.size_in_bytes, sizeof(aux_entrie.size_in_bytes), 1, system_file);
        fread(&aux_entrie.type, sizeof(aux_entrie.type), 1, system_file);
        fread(&aux_entrie.first_sector, sizeof(aux_entrie.first_sector), 1, system_file);

        fseek(system_file, this->bytes_per_sector+i*32+28, SEEK_SET);
        fread(&aux_entrie.amount_sectors, 4, 1, system_file);

        if (!strcmp(name_without_extension, aux_entrie.name)) {
          fseek(system_file, this->bytes_per_sector + i*32, SEEK_SET);
          char aux = 0xE5;
          fwrite(&aux, 1, 1, system_file);
          break;
        }
      }

      char reader;

      int first_time = 1;

      for(int i = 0; i <= (aux_entrie.amount_sectors / 8); i++) {
        fseek(system_file, this->get_bitmap_position() + (aux_entrie.first_sector/512) + i, SEEK_SET); // vai na posicao do BYTE onde o bitmap deve ser marcado
        fread(&reader, sizeof(reader), 1, system_file);

        if(i > 0){
          first_time = 0;
        }

        if((!((i) == (aux_entrie.amount_sectors+aux_entrie.first_sector) / 8) && !first_time)){ // se nao estiver nos ultimos setores e nao for a primeira vez
          for(int bit = 0; bit < 8; bit++){ // seta todos os bits
              reader ^= (1 << bit);
          }
        }
        else if((!((i) == (aux_entrie.amount_sectors+aux_entrie.first_sector) / 8) && first_time)){ // se nao estiver nos ultimos setores e for a primeira vez
          for(int bit = aux_entrie.first_sector % 8; bit < 8; bit++){ // seta os bits partindo de uma posicao possivelmente no meio do byte 
              reader ^= (1 << bit);
          }
        }
        else if(((i) == (aux_entrie.amount_sectors+aux_entrie.first_sector) / 8 && !first_time)){ // se estiver nos ultimos setores e nao for a primeira vez 
          for(int bit = 0; bit < (aux_entrie.amount_sectors+aux_entrie.first_sector) % 8; bit++){ // seta os bits do comeco do byte ate possivelmente no meio
              reader ^= (1 << bit); // o bit da posicao (aux_entrie.first_sector % 8 + bit) é alterado
          }
        }
        else if ((i) == (aux_entrie.amount_sectors+aux_entrie.first_sector) / 8 && first_time){ // se estiver nos ultimos setores e for a primeira vez 
            for(int bit = aux_entrie.first_sector % 8; bit < (aux_entrie.amount_sectors+aux_entrie.first_sector) % 8; bit++){ // seta partindo possivelmente do meio ate possivelmente no meio
              reader ^= (1 << bit);  
          }
        }
        fseek(system_file, this->get_bitmap_position() + (aux_entrie.first_sector/512) + i, SEEK_SET);
        // printf("Reader: %d\n", reader);

        fwrite(&reader, sizeof(reader), 1, system_file); // reescreve o byte com a alteração do bit
      }

      fclose(system_file);

      return 1;
    }
};

int main()
{
  char file_system_name[30], file_name[30] = "teste.txt", file_name2[30] = "teste2.txt";
  FILE *file;
  File_system file_system;
  int amount_of_sectors = 30, deu_certo, choice = 0;
  int escolha;

  do {
      cout << "|---------------------------------------------------|" << endl;
      cout << "|                  MENU PRINCIPAL                   |" << endl;
      cout << "|---------------------------------------------------|" << endl;
      cout << "|                                                   |" << endl;
      cout << "| 1. Criar novo sistema.                            |" << endl;
      cout << "| 2. Utilizar um sistema ja existente.              |" << endl;
      cout << "| 0. Sair                                           |" << endl;
      cout << "|                                                   |" << endl;
      cout << "|---------------------------------------------------|" << endl;
      cout << "Selecione uma opcao (numero correspondente) => ";
      cin >> escolha;

      switch (escolha) {
          case 1:{
                  cout << "\n|---------------------------------------------------|" << endl;
                  cout << "| - Nome do novo Sistema => ";
                  scanf("%s", file_system_name);
                  cout << "\n|---------------------------------------------------|" << endl;
                  cout << "\n|---------------------------------------------------|" << endl;
                  cout << "Quantidade de setores: ";
                  scanf("%d", &amount_of_sectors);
                  cout << "\n|---------------------------------------------------|" << endl;
                  file_system.file_system_not_created(file_system_name, amount_of_sectors);
                  int subEscolha;
                  do {
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "|                 SUBMENU - Opcao 1                 |" << endl;
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "|                                                   |" << endl;
                      cout << "| 1. Copiar do disco para o sistema.                |" << endl;
                      cout << "| 2. Copiar do sistema para o disco.                |" << endl;
                      cout << "| 3. Remover um arquivo do sistema.                 |" << endl;
                      cout << "| 4. Listar todos os arquivos do sistema.           |" << endl;
                      cout << "| 0. Voltar ao menu principal                       |" << endl;
                      cout << "|                                                   |" << endl;
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "Selecione uma opcao (numero correspondente) => ";
                      cin >> subEscolha;

                      switch (subEscolha) {
                          case 1:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser inserido => ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              deu_certo = file_system.copy_disk_to_system(file_name);

                              if (!deu_certo){
                                cout << "\n|---------------------------------------------------|" << endl;
                                cout << "| - Nao foi possivel inserir\n";
                                cout << "|---------------------------------------------------|" << endl;
                              }
                              cout << "\n";
                              break;
                          case 2:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser copiado para disco => ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              deu_certo = file_system.copy_system_to_disk(file_name);
                              break;
                          case 3:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser removido: ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              file_system.remove_file(file_name);
                              break;
                          case 4:
                              cout << endl;
                              file_system.list_files();
                              break;
                          case 0:
                              cout << "Voltando ao menu principal..." << endl;
                              break;
                          default:
                              cout << "Opcao invalida! Tente novamente." << endl;
                              break;
                      }
                  } while (subEscolha != 0);
              }
              break;
          case 2:{
              cout << "\n|---------------------------------------------------|" << endl;
              cout << "| - Nome do Sistema existente: ";
              scanf("%s", file_system_name);
              cout << "\n|---------------------------------------------------|" << endl;
              file_system.file_system_already_created(file_system_name);
              int subEscolha;
              do {
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "|                 SUBMENU - Opcao 2                 |" << endl;
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "|                                                   |" << endl;
                      cout << "| 1. Copiar do disco para o sistema.                |" << endl;
                      cout << "| 2. Copiar do sistema para o disco.                |" << endl;
                      cout << "| 3. Remover um arquivo do sistema.                 |" << endl;
                      cout << "| 4. Listar todos os arquivos do sistema.           |" << endl;
                      cout << "| 0. Voltar ao menu principal                       |" << endl;
                      cout << "|                                                   |" << endl;
                      cout << "|---------------------------------------------------|" << endl;
                      cout << "Selecione uma opcao (numero correspondente) => ";
                      cin >> subEscolha;

                      switch (subEscolha) {
                          case 1:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser inserido => ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              deu_certo = file_system.copy_disk_to_system(file_name);

                              if (!deu_certo){
                                cout << "\n|---------------------------------------------------|" << endl;
                                cout << "| - Nao foi possivel inserir\n";
                                cout << "|---------------------------------------------------|" << endl;
                              }
                              cout << "\n";
                              break;
                          case 2:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser copiado para disco => ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              deu_certo = file_system.copy_system_to_disk(file_name);
                              break;
                          case 3:
                              cout << "\n|---------------------------------------------------|" << endl;
                              cout << "\n| - Nome do arquivo a ser removido: ";
                              scanf("%s", file_name);
                              cout << "\n|---------------------------------------------------|" << endl;
                              file_system.remove_file(file_name);
                              break;
                          case 4:
                              cout << endl;
                              file_system.list_files();
                              break;
                          case 0:
                              cout << "Voltando ao menu principal..." << endl;
                              break;
                          default:
                              cout << "Opção invalida! Tente novamente." << endl;
                              break;
                      }
                  } while (subEscolha != 0);
              }
              break;
          case 0:
              cout << "Saindo do sistema..." << endl;
              break;
          default:
              cout << "Opção invalida! Tente novamente." << endl;
              limparBuffer();
              break;
      }

  } while (escolha != 0);

  return 0;
}