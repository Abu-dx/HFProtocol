// Viterbi.h: interface for the Viterbi class.
//
#pragma once

#define Kc	7		/* constraint length */
#define MAXINT   16384

// K == 7        /* polynomials for K = 7 */
// int g[2][K] = {{1,  1, 1, 1,  0, 0, 1},  /* 171 */
//               {1,  0, 1, 1,  0, 1, 1}}; /* 133 */

class Viterbi  
{
public:
	Viterbi(int g[2][Kc]);
	virtual ~Viterbi();

	void Decode(byte *channel_output_vector, int channel_length,byte *decoder_output_matrix,int *out_length);
protected:
	int nxt_stat(int current_state, int input, int *memory_contents);
	void deci2bin(int d, int size, int *b);
	int bin2deci(int *b, int size);
	void init_quantizer(void);
	int soft_quant(byte channel_symbol);
	int soft_metric(int data, int guess);

protected:
	int TWOTOTHEM;     /* 2^(K - 1) -- change as required */
	int number_of_states;
	int depth_of_trellis;
	long t;

	int quantizer_table[256]; 

	int *memory_contents;                   /* input + conv. encoder sr */ 
    int **input;					/* maps current/nxt sts to input */ 
    int **output;					 /* gives conv. encoder output */ 
    int **nextstate;				/* for current st, gives nxt given input */ 
    int **accum_err_metric;			/* accumulated error metrics */ 
    int **state_history;			/* state history table */ 
    int *state_sequence;            /* state sequence list */ 
	int binary_output[2];                     /* vector to store binary enc output */ 
    int branch_output[2];                     /* vector to store trial enc output */ 
};
