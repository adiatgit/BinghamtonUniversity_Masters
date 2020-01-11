/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES = 1;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */

APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 16); //changing 32 -> 16 as there are only 16 AR in problem statement
  memset(cpu->regs_valid, 1, sizeof(int) * 16); //for same reason as above
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);
  cpu->robfrontptr =0;
  cpu->robrearptr= -1;
  cpu->robitemcount = 0;
  cpu->clock =1;
  cpu->is_halted = 0;
  cpu->zflag_valid_bit = 0;
  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }
  for (int i = 0; i < 16; ++i) {
    cpu->regs_valid[i] = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}


void enqueue(int *intArray,int *front,int *rear, int *itemCount, int size,int data) {

      if(*rear == size-1) {
         *rear = -1;
      }
      *rear =*rear+1;
      intArray[*rear] = data;
      *itemCount= *itemCount+1;
}

int dequeue(int *intArray,int *front,int *rear, int *itemCount, int size){
   int data = intArray[*front];
   *front = *front+1;

   if(*front == size){
      *front = 0;
   }
   *itemCount = *itemCount - 1;
   return data;
}


/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */

// adjustRob(int array[], int *robptr){
//   for(int i=0; i< sizeof(array)/sizeof(array[0]); i++){
//     for(int j=robfrontptr;j < robrearptr; j++){
//       if(array[i] == robptr[j]){
//         cpu->rob[j] = cpu->rob[j+1];
//       }
//     }
//   }
// }
int deleteElementRob(int array[],int arrsize,int *rob, int *frontptr,int *rearptr,int *itemcount){
//printf("\n value of queues are %d %d %d \n",*frontptr,*rearptr, *itemcount);
for(int k = 0; k< arrsize; k++){
  int i;
  for(i = *frontptr; i <= *rearptr; i++){
  //    printf("\n %d - %d - %d\n",array[k],rob[i],i);
      if(rob[i] == array[k]){
        break;
      }
  }
//  printf("Value of i is %d",i);
  //printf("\n == found element in array element: %d rob element:%d robptr: %d rearptr: %d frontptr: %d itemcount: %d\n",array[k],rob[i],i,*rearptr,*frontptr, *itemcount);
  if(i <= (*rearptr)){
    for(int j = i; j < *rearptr; j++){
      rob[j] = rob[j+1];
    }
    *rearptr = *rearptr-1;
    *itemcount = *itemcount -1;
  }
  }
  return *itemcount;
}
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}
int
get_pc(int index)
{
  return (index*4 + 4000);
}
// void adjustArray(int *arrptr, int arrsize){
// int i;
//   for(i=0; i< arrsize; i++){
//     if(arrptr[i] == 0){
//       break;
//     }
//   }
//     if(i < arrsize)
//     {
//       arrsize = arrsize - 1;
//       for(int j=i; j<arrsize; j++){
//          arrptr[j] = arrptr[j+1];
//        }
//     }
//   }
//
// int adjustPipeline(int *arrrob,int n, int cpupc, int stagepc, int memsize){
//   int arr2[(cpupc - stagepc)/4];
//   int j=0;
//   if(cpupc > stagepc){
//     for(int i=stagepc+4; i<=cpupc; i=i+4){ //inserting pc to be deleted in the arr2 and decreasing memsize accordingly
//       arr2[j] = i;
//       j++;
//       memsize--;
//     }
//   }
//   else {
//     for(int i=cpupc; i<=stagepc; i=i+4){
//       arr2[j] = i; j++;
//       memsize++;
//     }
//   }
  // printf("\n Printing the instructions to be deleted \n");
  // for(int k=0;k<sizeof(arr2)/sizeof(arr2[0]); k++){printf("%d ",arr2[k]);}

//     void deleteRobEntry(int *list){
//     printf("\n Printing elements inside rob \n");
//     for(int m =0; m< n; m++){
//       printf("%d ",arrrob[m]);
//     }
//     printf("\n Size of rob array is %d\n",n);
//     printf("\n Size of array to deleted is %d \n", (int)(sizeof(list)/sizeof(list[0])));
//     for(int k = 0;k<(sizeof(list)/sizeof(list[0]));k++){
//       int i;
//       for(i=0; i< n; i++){
//         if(arrrob[i] == list[k]){
//           printf("found element %d", list[k]);
//           break;
//         }
//       }
//
//       if(i < n)
//       {
//         n = n - 1;
//         printf("\n ---------Inside the loop %d %d\n",i,n);
//         for(int j=i; j<n; j++){
//            arrrob[j] = arrrob[j+1];
//          }
//       }
//     }
//     for(int i=0; i< sizeof(arrrob)/sizeof(arrrob[0]);i++){
//       printf("%d \n", arrrob[i]);
//     }
//   }
//   deleteRobEntry(arr2);
//   return memsize;
// }

static void
print_instruction(CPU_Stage* stage)
{
  if ((strcmp(stage->opcode, "ADDL") == 0) || (strcmp(stage->opcode, "SUBL") == 0) || (strcmp(stage->opcode, "LOAD") == 0)){
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  if((strcmp(stage->opcode, "STORE") == 0)){
      printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  if(strcmp(stage->opcode, "STR") == 0){
    printf("%s,R%d,R%d,R%d",stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  //##### Adding following code for register to register instructions
  if ((strcmp(stage->opcode, "ADD") == 0)|| (strcmp(stage->opcode, "AND") == 0) || (strcmp(stage->opcode, "SUB") == 0) ||
  (strcmp(stage->opcode, "OR") == 0) || (strcmp(stage->opcode, "EX-OR") == 0)
  || (strcmp(stage->opcode, "MUL") == 0) || (strcmp(stage->opcode, "LDR") == 0)) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd,stage->rs1, stage->rs2);
  }
  /* Control Flow instruction */
  if((strcmp(stage->opcode, "BZ") == 0) || (strcmp(stage->opcode, "BNZ") == 0)){
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  if(strcmp(stage->opcode, "JUMP") == 0){
    printf("%s,R%d, #%d ", stage->opcode, stage->rs1,stage->imm);
  }
  if(strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
    //let instructions already in pipeline to be completed
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}
static void print_stage_content_anyways(char* pos, char* name, CPU_Stage* stage, int cpupc, APEX_CPU* cpu)
{
  for(int i=cpu->robfrontptr; i<= cpu->robrearptr; i++){
    printf("%d-", cpu->rob[i]);}
    printf("\n");
  printf("%-7s---%-10s:pc(%-4d),cpu(%d)|%-1s:%-2d|%-5s:%-2d|%-8s:%-2d|%-3s:%-2d|rs1:%-2d|rs2:%-2d|rd:%-2d|imm:%-2d|rs1_val:%-2d|rs2_val:%-2d|buf:%-2d|fp:%-2d|rp:%-2d|rC:%-2d",pos,name, stage->pc,cpupc, "Z",cpu->zflag,"Busy",stage->busy,"stalled", stage->stalled, "equ",(!stage->busy && !stage->stalled),stage->rs1,stage->rs2,stage->rd,
   stage->imm,stage->rs1_value,stage->rs2_value, stage->buffer, cpu->robfrontptr, cpu->robrearptr, cpu->robitemcount);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  CPU_Stage* drf_stage = &cpu->stage[DRF];


  if(cpu->is_halted ==0){
  if(cpu->bz_branch_taken == 0){
  //print_stage_content_anyways("Before","Fetch", stage, cpu->pc,cpu);
    //  printf("\n --- Fetch stage ---  Busy:  %d, stalled: %d , equation: %d, stage_pc : %d, cpu_pc :%d \n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;
    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    if((strlen(current_ins->opcode) > 0)){
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    if(strcmp(stage->opcode, "STR") == 0){
    stage->rs3 = current_ins->rs3;
    }
    /*Keep order of instruction dispatched */
    if((cpu->rob[(cpu->robrearptr)] != cpu->pc) & (strlen(current_ins->opcode) > 0)){
      enqueue(cpu->rob,&(cpu->robfrontptr), &(cpu->robrearptr), &(cpu->robitemcount),100,cpu->pc);

    /* Update PC for next instruction */
    cpu->pc += 4;

    //stalling logic
    if((strcmp(stage->opcode, "STR") == 0)){
      if((cpu->regs_valid[stage->rs1] == 0) || (cpu->regs_valid[stage->rs2] == 0) || (cpu->regs_valid[stage->rs3] == 0)){
        stage->stalled = 1;
        drf_stage->stalled =1;
    }
  }
    else{
    if(cpu->regs_valid[stage->rs1] == 0 || cpu->regs_valid[stage->rs2] == 0){
      if((strcmp(stage->opcode, "MOVC") != 0) & (strcmp(stage->opcode, "BZ") != 0) & (strcmp(stage->opcode, "BNZ") != 0)){
        stage->stalled = 1;
        drf_stage->stalled =1;
    //    printf("\n Stalling pipeline ====== \n");
      }
    }
  }
    if(strcmp(stage->opcode, "MOVC") == 0){ //checking if destination register of movc is in pipeline
      if(cpu->regs_valid[stage->rd] == 0){
          stage->stalled = 1;
        }
    }

    //stall pipeline when immediate next prior instruction is arithematic
    if((strcmp(stage->opcode, "BZ") == 0) || (strcmp(stage->opcode, "BNZ") == 0)){
      if((strcmp(drf_stage->opcode, "ADD") == 0) || (strcmp(drf_stage->opcode, "ADDL") == 0) || (strcmp(drf_stage->opcode, "MUL") == 0) ||
       (strcmp(drf_stage->opcode, "SUB") == 0) || (strcmp(drf_stage->opcode, "SUBL") == 0)){
        stage->busy = 1;
      }
    }

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
}
}
}
}
  //print_stage_content_anyways("After","Fetch", stage, cpu->pc,cpu);
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  CPU_Stage* f_stage = &cpu->stage[F];
  CPU_Stage* ex1_stage = &cpu->stage[EX1];
  if(stage->pc == ex1_stage->pc){
    if(ENABLE_DEBUG_MESSAGES){printf("Decode RF: Empty\n");}
  }
   else if((strcmp(stage->opcode, "NOOP") == 0)){
     printf("Decode RF: NOOP \n");
  }
  else {
  //print_stage_content_anyways("Before","Decode", stage, cpu->pc,cpu);

//  printf("\n --- Decode stage --- Busy:  %d, stalled: %d , equation: %d, stage_pc:%d , cpu_pc: %d \n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled) {


    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }


    if((strcmp(stage->opcode, "ADD") == 0) || (strcmp(stage->opcode, "LDR") == 0) || (strcmp(stage->opcode, "MUL") == 0)
    || (strcmp(stage->opcode, "SUB") == 0)
    || (strcmp(stage->opcode, "AND") == 0) || (strcmp(stage->opcode, "OR") == 0) || (strcmp(stage->opcode, "EX-OR") == 0)) {
       stage->rs1_value = cpu->regs[stage->rs1];
       stage->rs2_value = cpu->regs[stage->rs2];
    }

    if ((strcmp(stage->opcode, "ADDL") == 0) || (strcmp(stage->opcode, "SUBL") == 0 ) || (strcmp(stage->opcode, "LOAD") == 0)) {
       stage->rs1_value = cpu->regs[stage->rs1];
    }

    if(strcmp(stage->opcode, "STR") == 0){
       stage->rs1_value = cpu->regs[stage->rs1];
       stage->rs2_value = cpu->regs[stage->rs2];
       stage->rs3_value = cpu->regs[stage->rs3];
    }
    if (strcmp(stage->opcode, "JUMP") == 0){
       stage->rs1_value = cpu->regs[stage->rs1];
    }

    if(strcmp(stage->opcode, "HALT") == 0){
      f_stage->stalled = 1;
      f_stage->stalled =1;
      cpu->is_halted =1;
      // f_stage->busy =1;
      // f_stage->stalled = 1;
      // f_stage->busy =1;
    }
    if((strcmp(stage->opcode, "BZ") == 0) || (strcmp(stage->opcode, "BNZ") == 0)) {
      cpu->zflag_valid_bit =1;
    }
    if((strcmp(stage->opcode, "BZ") != 0) & (strcmp(stage->opcode, "BNZ") != 0) & (strcmp(stage->opcode, "HALT") != 0) & (strcmp(stage->opcode, "STR") != 0) & (strcmp(stage->opcode, "STORE") != 0)){
      cpu->regs_valid[stage->rd] = 0;
    }
    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  }
}
  //print_stage_content_anyways("After","Decode", stage, cpu->pc,cpu);
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  CPU_Stage* ex2_stage = &cpu->stage[EX2];
  if(stage->pc == ex2_stage->pc){
    if(ENABLE_DEBUG_MESSAGES){printf("Execute 1:Empty\n");}
  }
  else if((strcmp(stage->opcode, "NOOP") == 0)){
    //do nothing
 }

  else{
  //print_stage_content_anyways("Before","Execute1", stage, cpu->pc,cpu);

  //  printf("\n--- Execute 1 Stage --- Busy:  %d, stalled: %d , equation: %d , stage_pc: %d, cpu_pc: %d \n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled) {

    /*################## Add */
    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute 1", stage);
    }
  }
}
  //print_stage_content_anyways("After","Execute1", stage, cpu->pc,cpu);
  return 0;
}

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  CPU_Stage* drf_stage = &cpu->stage[DRF];
  CPU_Stage* f_stage = &cpu->stage[F];
  CPU_Stage* ex1_stage = &cpu->stage[EX1];
  CPU_Stage* mem1_stage = &cpu->stage[MEM1];

  if(mem1_stage->pc == stage->pc){
    if(ENABLE_DEBUG_MESSAGES){printf("Execute 2: Empty\n");}
  }
  else if((strcmp(stage->opcode, "NOOP") == 0)){
    printf("Execute 2: NOOP \n");
 }

  else{
  //print_stage_content_anyways("Before","Execute2", stage, cpu->pc,cpu);

  //CPU_Stage* stage_drf = &cpu->stage[DRF];
  //printf("\n -- Execute 2 stage -- Busy:  %d, stalled: %d , equation: %d, stage_pc :%d, cpu_pc: %d\n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled){

    //forwarding the data for non load store instructions from execute 2
   if((strcmp(stage->opcode, "ADD") == 0) || (strcmp(stage->opcode, "OR") == 0)) {
       stage->buffer = stage->rs1_value + stage->rs2_value;
   }
   if(strcmp(stage->opcode, "SUB") == 0) {
       stage->buffer = stage->rs1_value - stage->rs2_value;
   }
   if(strcmp(stage->opcode, "ADDL") == 0) {
       stage->buffer = stage->rs1_value + stage->imm;
   }
   if(strcmp(stage->opcode, "SUBL") == 0) {
       stage->buffer = stage->rs1_value - stage->imm;
   }
   if ((strcmp(stage->opcode, "MUL") == 0) || (strcmp(stage->opcode, "AND") == 0)) {
       stage->buffer = stage->rs1_value * stage->rs2_value;
   }
   if(strcmp(stage->opcode, "EX-OR") == 0){
            stage->buffer = (stage->rs1_value * (1- stage->rs1_value)) + (stage->rs2_value * (1- stage->rs2_value));
   }
   if (strcmp(stage->opcode, "LDR") == 0){
       stage->mem_address = stage->rs1_value + stage->rs2_value; //calculated address
   }
   /* Store */
   if ((strcmp(stage->opcode, "STORE") == 0)){
     stage->mem_address = stage->rs2_value + stage->imm; //calculated address
   }
  if(strcmp(stage->opcode, "LOAD") == 0){
    stage->mem_address = stage->rs1_value + stage->imm;
  }
   if (strcmp(stage->opcode, "STR") == 0){
     stage->mem_address = stage->rs3_value + stage->rs2_value; //calculated address
   }

   /* MOVC */
   if (strcmp(stage->opcode, "MOVC") == 0) {
     stage->buffer = stage->imm + 0;
   }



    // control instructions
    if(((strcmp(stage->opcode, "BNZ") == 0) & (cpu->zflag==0) & (cpu->zflag_valid_bit=1)) || ((strcmp(stage->opcode, "BZ") == 0) & (cpu->zflag==1) & (cpu->zflag_valid_bit==1))) {
          //f_stage->stalled = 1;
          int address =   get_code_index(stage->pc) + ((stage->imm)/4);
          cpu->pc = get_pc(address);
          int arraysize;
          int k=0;
          if((f_stage->pc) > (stage->pc)){
            arraysize = (((f_stage->pc) -(stage->pc))/4);
          }
          else{
            arraysize = (((stage->pc) -(f_stage->pc))/4);
          }
          int array[arraysize];
          if((f_stage->pc) > (stage->pc)){
            for(int i=stage->pc+4;  i <= f_stage->pc ; i=i+4){
              //printf("%d \n", array[k]);
              array[k] = i;
              k++;
            }
          }
          else{
            for(int i=f_stage->pc;i < stage->pc ; i=i+4){
              //printf("%d \n", array[k]);
              array[k] = i;
              k++;
            }
          }
          // printf("Printing the elements in array to be deleted \n");
          // for(int i=0; i< arraysize; i++){
          //   printf("%d \n", array[i]);
          // }
          deleteElementRob(array,arraysize,cpu->rob,&(cpu->robfrontptr), &(cpu->robrearptr), &(cpu->robitemcount));
          cpu->zflag=0;
          cpu->zflag_valid_bit =0;
          cpu->bz_branch_taken =1;
          strcpy(ex1_stage->opcode, "NOOP");
          strcpy(drf_stage->opcode,"NOOP");
          strcpy(f_stage->opcode,"NOOP");
          drf_stage->busy=1;
          //f_stage->busy = 1;
    }

    if(strcmp(stage->opcode, "JUMP") == 0){
      f_stage->stalled = 0;
      stage->buffer = stage->imm + stage->rs1_value;
      cpu->bz_branch_taken =1;

      cpu->pc = get_pc(stage->buffer);
      int arraysize;
      int k=0;
      if((f_stage->pc) > (stage->pc)){
        arraysize = (((f_stage->pc) -(stage->pc))/4);
      }
      else{
        arraysize = (((stage->pc) -(f_stage->pc))/4);
      }
      int array[arraysize];
      if((f_stage->pc) > (stage->pc)){
        for(int i=stage->pc+4;  i <= f_stage->pc ; i=i+4){
          printf("%d \n", array[k]);
          array[k] = i;
          k++;
        }
      }
      else{
        for(int i=f_stage->pc;i < stage->pc ; i=i+4){
          printf("%d \n", array[k]);
          array[k] = i;
          k++;
        }
      }

      deleteElementRob(array,arraysize,cpu->rob,&(cpu->robfrontptr), &(cpu->robrearptr), &(cpu->robitemcount));
      strcpy(ex1_stage->opcode, "NOOP");
      strcpy(drf_stage->opcode,"NOOP");
      strcpy(f_stage->opcode,"NOOP");
      drf_stage->busy=1;

    }



    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute 2", stage);
    }

}
}
//print_stage_content_anyways("After","Execute2", stage, cpu->pc,cpu);

return 0;
}


/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  CPU_Stage* mem1_stage = &cpu->stage[MEM2];
  if(mem1_stage->pc == stage->pc){
    if(ENABLE_DEBUG_MESSAGES){printf("Memory 1: Empty\n");}
  }
  else if((strcmp(stage->opcode, "NOOP") == 0)){
         printf("Memory 1: NOOP \n");
 }
  else{
  //print_stage_content_anyways("Before","Memory1", stage, cpu->pc,cpu);

  //printf("\n  ---Memory 1 stage---. Busy:  %d, stalled: %d , equation: %d, stage_pc: %d, cpu_pc: %d\n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled) {


    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory 1", stage);
    }
  }
}
  //print_stage_content_anyways("After","Memory 1", stage, cpu->pc,cpu);
  return 0;
}

int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  // CPU_Stage* drf_stage = &cpu->stage[DRF];
  // CPU_Stage* f_stage = &cpu->stage[F];
  CPU_Stage* wb_stage = &cpu->stage[WB];
  if(wb_stage->pc == stage->pc){
    if(ENABLE_DEBUG_MESSAGES){printf("Memory 2: Empty\n");}
  }
  else if((strcmp(stage->opcode, "NOOP") == 0)){
  printf("Memory 2:NOOP\n");
 }
  else{
  //print_stage_content_anyways("Before","Memory2", stage, cpu->pc,cpu);

  //printf("\n --- Memory 2 stage ---. Busy:  %d, stalled: %d , equation: %d, stage_pc: %d, cpu_pc: %d \n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  //CPU_Stage* stage_drf = &cpu->stage[DRF];
  if (!stage->busy && !stage->stalled) {
    /*Checking if the instruction is register to register*/
    if((strcmp(stage->opcode, "ADD") == 0) || (strcmp(stage->opcode, "ADDL") == 0) || (strcmp(stage->opcode, "SUB") == 0) || (strcmp(stage->opcode, "SUBL") == 0) || (strcmp(stage->opcode, "AND") == 0) ||
   (strcmp(stage->opcode, "EX-OR") == 0) || (strcmp(stage->opcode, "OR") == 0) || (strcmp(stage->opcode, "MOVC") == 0) || (strcmp(stage->opcode, "MUL") == 0)){
     //Doesn't have to do anything.
   }

   else{

      /* ####################LDR */
      if ((strcmp(stage->opcode, "LDR") == 0) || (strcmp(stage->opcode, "LOAD") == 0)) {
        stage->buffer = cpu->data_memory[stage->mem_address];
      }
      if ((strcmp(stage->opcode, "STR") == 0)){
        cpu->data_memory[stage->mem_address] = stage->rs1_value;
      }
      if((strcmp(stage->opcode, "STORE") == 0)){
        cpu->data_memory[stage->mem_address] = stage->rs1_value;
      }
    }





    /* Copy data from memory 2 latch to writeback latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory 2", stage);
    }
  }
}
  //print_stage_content_anyways("After","Memory 2", stage, cpu->pc,cpu);
  return 0;
}
/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  CPU_Stage* drf_stage = &cpu->stage[DRF];
  CPU_Stage* f_stage = &cpu->stage[F];
  CPU_Stage* ex2_stage = &cpu->stage[EX1];
  // CPU_Stage* mem2_stage = &cpu->stage[MEM2];
  cpu->bz_branch_taken = 0;
  //print_stage_content_anyways("Before","Writeback", stage, cpu->pc,cpu);

 if((strcmp(stage->opcode, "NOOP") == 0)){
       printf("Writeback RF: NOOP \n");
 }
else{
  //printf("\n --- WB stage ---. Busy:  %d, stalled: %d , equation: %d, stage_pc: %d, cpu_pc:%d\n",stage->busy,stage->stalled,(!stage->busy && !stage->stalled), stage->pc, cpu->pc);
  if (!stage->busy && !stage->stalled){

    if(cpu->rob[cpu->robfrontptr] == stage->pc){
      if((strcmp(stage->opcode, "MOVC") == 0) || (strcmp(stage->opcode, "ADD") == 0) || (strcmp(stage->opcode, "MUL") == 0) || (strcmp(stage->opcode, "EX-OR") == 0) ||
      (strcmp(stage->opcode, "LDR") == 0) || (strcmp(stage->opcode, "AND") == 0) || (strcmp(stage->opcode, "OR") == 0)
      || (strcmp(stage->opcode, "ADDL") == 0) || (strcmp(stage->opcode, "SUB") == 0) || (strcmp(stage->opcode, "SUBL") == 0) || (strcmp(drf_stage->opcode, "LOAD") == 0)){
        //printf("\n ******************************** Changing values ***********************************\n");

        /* Update register file */
          cpu->regs[stage->rd] = stage->buffer;
          cpu->regs_valid[stage->rd] = 1;

        if(strcmp(stage->opcode, "MOVC") != 0){
          if(stage->buffer == 0){
            cpu->zflag = 1;
            //setting zero flag  if output is zero of arithematic instruction
          //  printf("\n +++++++Printing instruction that is setting zero flag %s %d\n",stage->opcode, stage->pc);
          }
        }
      }


    if((strcmp(drf_stage->opcode, "HALT") == 0)){
      //don't unstall the pipeline

    }
    else if((strcmp(drf_stage->opcode, "BNZ") == 0) || (strcmp(drf_stage->opcode, "BZ") == 0)){
      if(stage->pc == drf_stage->pc-4){
        // unstall when the next arithematic instruction writes back value in WB stage
        drf_stage->busy = 0;
        f_stage->busy = 0;
      }
    }
    else{
      if((strcmp(drf_stage->opcode, "STR") == 0)){
        if((cpu->regs_valid[drf_stage->rs1] ==1) & (cpu->regs_valid[drf_stage->rs2]==1) & (cpu->regs_valid[drf_stage->rs3]==1)){
          drf_stage->stalled = 0;
          f_stage->stalled = 0;
      }
}
else {
      if((cpu->regs_valid[drf_stage->rs1] ==1) & (cpu->regs_valid[drf_stage->rs2]==1)){
        //normal unstall when both operand registers are available/valid
        drf_stage->stalled = 0;
        f_stage->stalled = 0;
      }
    }
    }


      cpu->ins_completed++;
      dequeue(cpu->rob,&(cpu->robfrontptr), &(cpu->robrearptr), &(cpu->robitemcount),100);

      if (ENABLE_DEBUG_MESSAGES) {
        print_stage_content("Writeback", stage);
      }
    }
    else{
      printf("Writeback : Empty\n");
    }

  }
}
if((strcmp(stage->opcode, "HALT") == 0)){
cpu->robfrontptr = cpu->robrearptr +1;
}

  //print_stage_content_anyways("After","Writeback", stage, cpu->pc,cpu);
  return 0;
}


/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu, const char* task, int cycles_simulate)
{
  while (1) {

    /* All the instructions committed, so exit */
    if(strcmp(task, "simulate") == 0){
        ENABLE_DEBUG_MESSAGES=0;
      }
      else {
        ENABLE_DEBUG_MESSAGES =1;
      }

    if(((cpu->robrearptr == cpu->robfrontptr -1) && (cpu->pc > 4000)) || (cpu->clock > cycles_simulate)){

        printf("\n-- Contents of register file --\n");
        for(int i = 0; i< 16; i++){
          printf("REG[%-2d] - Value = %-6d - status %-3d \n",i,cpu->regs[i],cpu->regs_valid[i]);
        }


          printf("\n-- Contents of Data memory --\n");
          for(int i = 0; i< 100; i++){
              printf("MEM[%-2d] -  Data Value = %-6d \n",i,cpu->data_memory[i]);
          }
      break;
      }



    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d and pc value : %d \n", cpu->clock, cpu->pc);
      printf("--------------------------------\n");
    }
    // printf("Contents of the flags in cpu\n");
    // printf("Stage --Busy -- Stalled -- Combined\n");
    // for (int i = 0; i < NUM_STAGES; ++i) {
    //   printf("%d - %d - %d - %d \n",i,cpu->stage[i].busy, cpu->stage[i].stalled, (!cpu->stage[i].busy && !cpu->stage[i].stalled));
    // }
    if (ENABLE_DEBUG_MESSAGES) {
    printf("\n--- Cycles and task are %s and %d  \n",task,cycles_simulate);
    printf("\n Contents of the register file \n");
    for(int i = 0; i< 16; i++){ printf("%d - %d - %d \n",i,cpu->regs[i],cpu->regs_valid[i]);}
    printf("\n");
}
    writeback(cpu);
    memory2(cpu);
    memory1(cpu);
    execute2(cpu);
    execute1(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

  }

  return 0;
}
