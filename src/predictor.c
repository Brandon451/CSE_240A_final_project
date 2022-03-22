//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Brandon Saldanha, Mihir Pratap Singh";
const char *studentID   = "A59011109, A59011168";
const char *email       = "bsaldanha@ucsd.edu, m8singh@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Perceptron" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//? Add your own Branch Predictor data structures here

//Gshare
uint8_t *bht_gshare;
uint64_t ghistory;

//Tournamnet
uint8_t *gpt;
uint8_t *cpt;
uint8_t *lpt;
uint8_t *lht;
uint64_t phistory;

//Perceptron
int **perceptrons;
int perceptron_history_length;
int n_perceptrons;
int threshold;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//


//Fuction declarations:
void init_gshare();
void train_gshare(uint32_t pc, uint8_t outcome);
uint8_t make_prediction_gshare(uint32_t pc);

void init_tourn();
void train_tourn(uint32_t pc, uint8_t outcome);
uint8_t make_prediction_tourn(uint32_t pc);
uint8_t findChoice(uint32_t pc);

// Initialize the predictor
//
void init_predictor()
{
	//? Initialize Branch Predictor Data Structures
	switch (bpType){
		case STATIC:
			break;
		case GSHARE:
			init_gshare();
			break;
		case TOURNAMENT:
			init_tourn();
			break;
		case PERCEPTRON:
			//init_custom();
			break;
		default:
			break;
	}

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
{
  	//
  	//TODO: Implement prediction scheme

  	switch (bpType) {
    	case STATIC:
    		return TAKEN;
    	case GSHARE:
			return make_prediction_gshare(pc);
    	case TOURNAMENT:
			return make_prediction_tourn(pc);
    	case PERCEPTRON:
			//return make_prediction_custom(pc);
    	default:
    		return NOTTAKEN;
  	}

  	// If there is not a compatable bpType then return NOTTAKEN
  	return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint8_t outcome)
{
  	//
  	//? Implement Predictor training
	switch (bpType){
		case STATIC:
			break;
		case GSHARE:
			train_gshare(pc, outcome);
			break;
		case TOURNAMENT:
			train_tourn(pc, outcome);
			break;
		case PERCEPTRON:
			//train_custom();
			break;
		default:
			break;
	}
}


//*Gshare functions:

void init_gshare(){
  // Allocate an array to use as BHT
  // Size of BHT depends on number of bits used 
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t make_prediction_gshare(uint32_t pc){
  //get lower ghistoryBits of pc
  	uint32_t mask = ~(UINT32_MAX << ghistoryBits);
	uint32_t pcLowerBits = pc & mask;
	uint32_t ghistoryLowerBits = ghistory & mask;
	uint32_t index = ghistoryLowerBits ^ pcLowerBits;

	// uint32_t bht_entries = 1 << ghistoryBits;
  	// uint32_t pc_lower_bits = pc & (bht_entries-1);
  	// uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  	// uint32_t index = pc_lower_bits ^ ghistory_lower_bits;


  // if(verbose){
  //   printf("bht_entries: %d, pc_lower_bits: %d, ghistory: %lu, index: %d \n", bht_entries, pc_lower_bits, ghistory, index);
  // }
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

//GSHARE Training
void train_gshare(uint32_t pc, uint8_t outcome){
	uint32_t mask = ~(UINT32_MAX << ghistoryBits);
	uint32_t pcLowerBits = pc & mask;
	uint32_t ghistoryLowerBits = ghistory & mask;
	uint32_t index = ghistoryLowerBits ^ pcLowerBits;

	// uint32_t bht_entries = 1 << ghistoryBits;
  	// uint32_t pc_lower_bits = pc & (bht_entries-1);
  	// uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  	// uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

	//Update counters
	switch (bht_gshare[index])
	{
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void cleanup_gshare(){
  free(bht_gshare);
}


// * Tounrmanent functions

void init_tourn(){

	int g_entries = 1 << ghistoryBits;
	int lpt_entries = 1 << lhistoryBits;
	int lht_entries = 1 << pcIndexBits;
	gpt = (uint8_t*)malloc(g_entries * sizeof(uint8_t));
	cpt = (uint8_t*)malloc(g_entries * sizeof(uint8_t));
	lpt = (uint8_t*)malloc(lpt_entries * sizeof(uint8_t));
	lht = (uint8_t*)malloc(lht_entries * sizeof(uint8_t));
	phistory=0;

	int i = 0;
	for(i = 0; i< g_entries; i++){
		gpt[i] = WN;
		cpt[i] = WT;
	}
	for(i = 0; i< lpt_entries; i++){
		lpt[i] = WN;
	}
	for(i = 0; i< lht_entries; i++){
		lht[i] = 0;
	}
}


uint8_t findChoice(uint32_t pc){

	uint32_t andbits = 1<<pcIndexBits;
	uint32_t pc_lower_bits = pc&(andbits-1);
	uint32_t l_val = lpt[lht[pc_lower_bits]];
	uint32_t g_val = gpt[phistory];

	uint32_t g_pred = NOTTAKEN;
	uint32_t l_pred = NOTTAKEN;
	//printf("%d %d\n",g_val,l_val);
	switch(l_val){
	case WN:
		l_pred= NOTTAKEN;
		break;
	case SN:
		 l_pred= NOTTAKEN;
		 break;
	case WT:
		l_pred= TAKEN;
		break;
	case ST:
		 l_pred=  TAKEN;
		 break;
	default:
		printf("Warning: Undefined state of entry in l_val!\n");
	}
		
	switch(g_val){
	case WN:
		g_pred= NOTTAKEN;
		break;
	case SN:
		 g_pred= NOTTAKEN;
		 break;
	case WT:
		g_pred= TAKEN;
		break;
	case ST:
		 g_pred=  TAKEN;
		 break;
	default:
		printf("Warning: Undefined state of entry in g_val\n");
	}

	uint32_t choice = (g_pred << 1) | l_pred;

	return choice;
}

uint8_t make_prediction_tourn(uint32_t pc){
	uint32_t choice = findChoice(pc);
	switch(choice){
	case WN:
		if(cpt[phistory]>1)
			return NOTTAKEN;
		else 
			return TAKEN;
	case SN:
		 return NOTTAKEN;
	case WT:
		if(cpt[phistory]>1)
			return TAKEN;
		else
			return NOTTAKEN;
	case ST:
		 return TAKEN;
	default:
		printf("Warning: Undefined state of entry in choice!\n");
		return NOTTAKEN;
		}
}

void train_tourn(uint32_t pc, uint8_t outcome){
	uint32_t choice= findChoice(pc);
	uint32_t andbits=1<<pcIndexBits;
	uint32_t pc_lower_bits=pc&(andbits-1);
	int g_entries = 1 << ghistoryBits;
	int lht_entries = 1 << pcIndexBits;

	if(outcome==TAKEN){
		if(lpt[lht[pc_lower_bits]]<3)
			lpt[lht[pc_lower_bits]]+=1;

		if(gpt[phistory]<3)
			gpt[phistory]+=1;
	}
    	
	if(outcome==NOTTAKEN){
		if(lpt[lht[pc_lower_bits]]>0)
			lpt[lht[pc_lower_bits]]-=1;

		if(gpt[phistory]>0)
			gpt[phistory]-=1;
	}
    	
	lht[pc_lower_bits]=(lht[pc_lower_bits]<<1) | outcome;
	lht[pc_lower_bits]= lht[pc_lower_bits] & (lht_entries -1);
    
	int pred_choice = cpt[phistory];	
	switch(choice){
    case WN:
    	if(outcome == 1){
    		cpt[phistory] = (pred_choice>0)?pred_choice-1:pred_choice;
    	}
    	else{
    	   cpt[phistory] = (pred_choice<3)?pred_choice+1:pred_choice;
    	}
      break;
    case SN:
      break;
    case WT:
    	if(outcome == 1){
    		cpt[phistory] = (pred_choice<3)?pred_choice+1:pred_choice;
    	}
    	else{
    	  cpt[phistory] = (pred_choice>0)?pred_choice-1:pred_choice;
    	}
      break;
    case ST:
      break;
    default:
      printf("Warning: Undefined state of entry in train!\n");
  }
	phistory = (phistory << 1) | outcome;
	phistory = phistory & (g_entries - 1);
}



void cleanup_tourn(){
  free(gpt);
  free(cpt);
  free(lpt);
  free(lht);
}


// * Perceptron functions

void init_perceptron(){
  	ghistory = 0;
  	threshold = (1.93*perceptron_history_length) + 14;
  	int i =0;
  	int j = 0;
  	perceptrons = (int**)malloc(n_perceptrons * sizeof(int*));
  	for(i= 0; i<n_perceptrons;i++){
    	perceptrons[i] = (int*)malloc((perceptron_history_length+1) * sizeof(int));
    	for(j = 0; j<perceptron_history_length+1; j++){
    		perceptrons[i][j] = 1;
    	}
  	}
}

int compute_perceptron_result(uint32_t pc){
  	int index = (pc>>2)%n_perceptrons;
  	int result = perceptrons[index][0];
  	int i = 0;
  	uint64_t temp = ghistory;
  	for(i = 1; i< perceptron_history_length+1; i++){
  	  	if((temp & (1<<i)) != 0){
  	  	  	result += perceptrons[index][i];
  	  	}
  	  	else{
  	  	  	result -= perceptrons[index][i];
  	  	}
  	}
  	return result;
}

uint8_t make_prediction_perceptron(uint32_t pc){
  	int result = compute_perceptron_result(pc);
  	return (result>0)?TAKEN:NOTTAKEN;
}

void train_perceptron(uint32_t pc, uint8_t outcome){
  	int index = (pc>>2)%n_perceptrons;
  	int result = compute_perceptron_result(pc);
  	int sign = (outcome == TAKEN)?1:-1;
  	int i = 0;
  	uint64_t temp = ghistory;
  	if((result>0 && outcome == NOTTAKEN) || abs(result) < threshold){
  	  	for(i = 1; i< perceptron_history_length+1; i++){
  	  	  	if((temp & (1<<i)) != 0){
  	  	  	  	perceptrons[index][i] = perceptrons[index][i] + sign;
  	  	  	}
  	  	  	else{
  	  	  	  	perceptrons[index][i] = perceptrons[index][i] - sign;
  	  	  	}
  	  	}
  	  	perceptrons[index][0] = perceptrons[index][0] + sign;
  	}
  	ghistory = ((ghistory << 1) | outcome); 
  	ghistory = ghistory & ((1<<perceptron_history_length)-1);
}

void cleanup_perceptron(){
  	int i = 0;
  	for(i= 0; i<n_perceptrons;i++){
  	  	free(perceptrons[i]);
  	}
  	free(perceptrons);
}



// Free any dynamically allocated Datastructures
void cleanup() {
  	switch (bpType) {
    case STATIC:
    	break;
    case GSHARE:
      	cleanup_gshare();
      	break;
    case TOURNAMENT:
      	cleanup_tourn();
      	break;
    //case TAGE:
    //   	cleanup_tage();
    //   	break;
    case PERCEPTRON:
      	cleanup_perceptron();
      	break;
    default:
      	break;
  }
}