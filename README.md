# Sistemas-de-arquivos

##Especificação Geral:
	A estruturação geral do sistema de arquivos (Tabela 1) está dividida em quatro áreas principais, sendo elas: boot record, diretório raiz, mapa de bits e setor de dados. Para definir o último setor do mapa de bits é necessário ter o valor de T, o mesmo é descoberto por um cálculo que será mostrado no tópico “Formatador”.

|Boot Record | Diretório Raiz | Bitmap | Dados |
0            1               13      13+T      fim

###Boot Record:
	Em discussão a equipe optou por deixar o Boot Record (Tabela 2) no primeiro setor (setor 0).

 | Deslocamento | Comprimento | Siginificado |
 | --- | --- | --- |
 | 0 | 4 | Bytes por setor |
 | 4 | 4 | Entradas Diretório raiz |
 | 8 | 4 | Tamanho Bitmap |

###Diretório Raiz:
	Para a implementação da estrutura do diretório, a equipe optou por realizar a de um único nível, ou seja, todos os arquivos ficarão armazenados nesse diretório, sem subdiretórios, essa escolha se dá pelo fato de conseguir maior velocidade de acesso. A estrutura do diretório raiz (Tabela 3) é simples e entrega informações essenciais de um arquivo.

| Deslocamento | Comprimento | Significado |
| --- | --- | --- |
| 0 | 13 | Nome |
| 13 | 3 | Extensão |
| 16 | 4 | Tamanho |
| 20 | 2 | Tipo de acesso |
| 22 | 4 | Primeiro setor |
| 26 | 6 | Número de setores |

##Funcionalidades:
###Formatador:
	Para iniciar a formatação será requerido do usuário a quantidade de setores da partição que será formatada, para melhoria de representação a letra N será usada para essa entrada. Como algumas informações em nosso sistema serão fixadas, por exemplo a quantidade de bytes por setor.

 -Setores_bitmap = ceil ((N - 14) / (Bytes_por_setor * 8))
 -Tamanho_bitmap = Setores_bitmap * Bytes_por_setor
 -Setores_area_dados = (N -14) - Setores_bitmap

  É importante frisar que o usuário tem um mínimo de 16(depende das contas anteriores) setores para que o nosso sistema de arquivos funcione, esse número foi escolhido por conta de 14 setores serem fixos para boot record e diretório raiz, e no mínimo um setor para cada mapa de bits e dados.

###Mapa de bits:
	Para o mapa de bits, será criado um vetor de bytes, porém a manipulação do vetor deverá ser feita bit a bit, possibilitando um endereçamento maior.

###Cópia:
####Disco rígido para sistema de arquivos:
	Para a realização dessa funcionalidade é necessário que o arquivo esteja na mesma área do sistema, para não ter a necessidade de buscar um arquivo externo. Para realizar essa cópia é necessário alguns passos:
 1-Verificar se existe espaço contíguo no bitmap;
 2-Adicionar os metadados no root directory, se não atingiu o limite de arquivos;
 3-Adiciona os dados no setor de dados;
 4-Realiza a marcação dos setores utilizados no mapa de bits;

####Sistemas de arquivos para disco rígido:
	Para realizar essa cópia é necessário que o usuário especifique o nome do arquivo dentro do sistema de arquivos o qual deseja copiar, após isso, realizaremos os seguintes passos:
 1-Seleciona o arquivo que deseja copiar;
 2-Cria um  arquivo de mesmo tipo;
 3-Verifica qual o primeiro setor dos dados do arquivo;
 4-Posiciona para o primeiro setor do arquivo 
 5-Por fim realizar a cópia dos dados do arquivo;

###Listagem de arquivos: 
####Processo de Listagem de Arquivos:
  Para listar os arquivos no sistema, é necessário realizar os seguintes passos:
 1-Pular o Boot Record: O primeiro setor é ignorado, pois é reservado para o boot record.
 2-Ler Registros do Diretório Raiz: Cada registro de arquivo no diretório raiz é lido e interpretado.
 3-Extrair Nome, Extensão e Tamanho: O nome, extensão e tamanho do arquivo são extraídos do registro e formatados para exibição ao usuário.
 4-Exibir Arquivos: O nome, extensão e tamanho dos arquivos respectivamente são listados para o usuário, com exceção de arquivos marcados como excluídos.

###Identificação de Arquivos Excluídos:
  Arquivos excluídos são identificados pelo valor hexadecimal E5 no campo de nome do registro. Esses arquivos não são listados para o usuário.

###Remoção: 
	Para a realização a exclusão é necessário seguir os seguintes passos:
 1-Identificação do Arquivo: O sistema precisa identificar qual arquivo o usuário deseja excluir. Isso pode ser feito através do nome do arquivo.
 2-Atualização do Diretório Raiz: O nome do arquivo no diretório raiz é marcado como "E5" para indicar que o arquivo está excluído.
 3-Localização do primeiro setor: No diretório raiz existe a informação do primeiro setor que o arquivo ocupa.
 4-Atualização do Mapa de Bits: O mapa de bits é atualizado para indicar que os setores do arquivo estão livres.
 5-Confirmação da Exclusão: O sistema informa ao usuário que a exclusão foi concluída com sucesso.


