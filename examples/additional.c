#include <stdio.h>
#include <syscall.h>

int atoi(char *str);

int atoi(char *str){
  int result = 0;
  bool negative = false;
  int i = 0;

  if(str[i] == '-'){
    negative = true;
    i++;
  }

  for(; str[i] >= '0' && str[i] <= '9'; i++){
    result = result * 10 + str[i] - '0';
  }

  if(negative){
    return -1 * result;
  }
  return result;
}

int
main (int argc, char *argv[]) 
{
  if(argc != 5){
    printf("check arguments\n");
    return EXIT_FAILURE;
  }

  printf("%d %d\n", fibonacci(atoi(argv[1])), 
  max_of_four_int(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));

  return EXIT_SUCCESS;
}