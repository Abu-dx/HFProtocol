// Viterbi.cpp: implementation of the Viterbi class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Viterbi.h"
#include <malloc.h>
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Viterbi::Viterbi(int g[2][Kc])
{
	int i,j,l;
	int next_state;
	int n=2;
	t = 0;
	TWOTOTHEM =(int) pow((float)2,(float)(Kc-1));
	/* m (memory length) = K - 1 */ 
    int m = Kc - 1; 
	/* number of states = 2^(K - 1) = 2^m for k = 1 */ 
    number_of_states = (int) pow((float)2, (float)(m)); 
    depth_of_trellis = Kc * 5;  // 认为时间经过 K * 5后没有相关性

	memory_contents = (int *)malloc(sizeof(int)*Kc);
	input = new int *[TWOTOTHEM];
	output = new int *[TWOTOTHEM];
	nextstate = new int *[TWOTOTHEM];
	accum_err_metric = new int *[TWOTOTHEM];
	
	
	for (i=0;i<TWOTOTHEM;i++)
	{
		input[i] = new int[TWOTOTHEM];
		output[i] = new int[n];
		nextstate[i] = new int[n];
		accum_err_metric[i] = new int[n];
		
	}
	/* initialize data structures */ 
	for (i=0;i<number_of_states;i++)
	{
		for (j = 0; j < number_of_states; j++) 
            input[i][j] = 0;    //  输入数据矩阵
		
        for (j = 0; j < n; j++)
		{ 
            nextstate[i][j] = 0;  //  状态转移矩阵
            output[i][j] = 0;     //  输出矩阵
        } 
		
        /* initial accum_error_metric[x][0] = zero */ 
        accum_err_metric[i][0] = 0; 
		accum_err_metric[i][1] = MAXINT; 
	}

	/*生成状态转移矩阵，输出矩阵，输入矩阵 */ 
	for (j = 0; j < number_of_states; j++)
	{ 
        for (l = 0; l < n; l++)
		{ 
            next_state = nxt_stat(j, l, memory_contents); // 初始状态为j，输入为l时的下一状态,同时返回寄存器中的值
            input[j][next_state] = l; 
 
            /* now compute the convolutional encoder output given the current 
               state number and the input value */ 
            branch_output[0] = 0; 
            branch_output[1] = 0; 
 
            for (i = 0; i < Kc; i++)
			{ 
                branch_output[0] ^= memory_contents[i] & g[0][i]; 
                branch_output[1] ^= memory_contents[i] & g[1][i]; 
            } 
 
            /* next state, given current state and input */ 
            nextstate[j][l] = next_state; 
            /* output in decimal, given current state and input */ 
            output[j][l] = bin2deci(branch_output, 2); 
        } 
    }
	init_quantizer();
}

Viterbi::~Viterbi()
{
	free(memory_contents);
	for (int i=0;i<TWOTOTHEM;i++)
	{
		delete input[i];
		delete output[i];
		delete accum_err_metric[i];
		delete nextstate[i];
	}
	delete input;
	delete output;
	delete accum_err_metric;
	delete nextstate;

}
/*the mean channel_symbol value is +/- 1
*/
void Viterbi::Decode(byte *channel_output_vector, int channel_length,byte *decoder_output_matrix,int *out_length)
{
	int i,j,l,step;
	int sh_ptr,sh_col,x,xx,h,hh;
	int branch_metric;
	int n = 2;
	int kt;
	int m_outlength=0;
	/* m (memory length) = K - 1 */ 
    int m = Kc - 1; 
	int *channel_output_matrix =(int *) malloc( channel_length * sizeof(int) ); 
	channel_length = channel_length / n; 


	state_sequence = new int[channel_length+1];
	state_history = new int *[TWOTOTHEM];
	for (i=0;i<TWOTOTHEM;i++)
	{
		state_history[i] = new int[channel_length+1];
		for (j = 0; j <= channel_length; j++)
		{ 
			state_history[i][j] = 0; 
		}
	}
	

	/* channel_output_matrix = reshape(channel_output, n, channel_length) */ 
    for (kt = 0; kt < (channel_length * n); kt += n)
	{ 
        for (i = 0; i < n; i++) 
            *(channel_output_matrix + (kt / n) + (i * channel_length) ) = soft_quant( *(channel_output_vector + (kt + i) ) ); 
    }
	for (t = 0; t < channel_length; t++)
	{
		if (t <= m) 
			step =(int) pow((float)2, (float)(m - t * 1)); 
		else
			step = 1; 
		/* set up the state history array pointer for this time t */ 
        sh_ptr = (int) ( ( t + 1 ) % (channel_length + 1) ); 
		/* repeat for each possible state */ 
        for (j = 0; j < number_of_states; j+= step)
		{
			/* repeat for each possible convolutional encoder output n-tuple */ 
            for (l = 0; l < n; l++)
			{
				branch_metric = 0; 
				deci2bin(output[j][l], n, binary_output); 
 
                /* compute branch metric per channel symbol, and sum for all 
                    channel symbols in the convolutional encoder output n-tuple */ 
                for (int ll = 0; ll < n; ll++)
				{ 
                    branch_metric = branch_metric + soft_metric( *(channel_output_matrix + 
                    ( ll * channel_length + t )), binary_output[ll] ); 
 
                } /* end of 'll' for loop */ 
				/* now choose the surviving path--the one with the smaller accumlated error metric... */ 
                if ( accum_err_metric[ nextstate[j][l] ] [1] > accum_err_metric[j][0] + branch_metric )
				{ 
                    /* save an accumulated metric value for the survivor state */ 
                    accum_err_metric[ nextstate[j][l] ] [1] = accum_err_metric[j][0] +  branch_metric; 
                    /* update the state_history array with the state number of the survivor */ 
                    state_history[ nextstate[j][l] ] [sh_ptr] = j; 
                } 
			}	
		}
		/* for all rows of accum_err_metric, move col 2 to col 1 and flag col 2 */ 
        for (j = 0; j < number_of_states; j++)
		{ 
            accum_err_metric[j][0] = accum_err_metric[j][1]; 
            accum_err_metric[j][1] = MAXINT; 
        } 
	} 
  
	/* now start the traceback, if we've filled the trellis */ 

		/* initialize the state_sequence vector--probably unnecessary */ 
		for (j = 0; j <= channel_length; j++) 
			state_sequence[j] = 0;  
		/* find the element of state_history with the min. accum. error metric */ 
		x = MAXINT; 
		for (j = 0; j < ( number_of_states / 2 ); j++)
		{ 

			if ( accum_err_metric[j][0] < accum_err_metric[number_of_states - 1 - j][0] )
			{ 
				xx = accum_err_metric[j][0]; 
				hh = j; 
			} 
			else
			{ 
				xx = accum_err_metric[number_of_states - 1 - j][0]; 
				hh = number_of_states - 1 - j; 
			} 
			if ( xx < x)  //  找出累积度量最小的
			{ 
				x = xx; 
				h = hh; 
			} 
		}
		/* now pick the starting point for traceback */ 
		state_sequence[channel_length] = h; // 从最小的累积度量状态往回找
		for (j = channel_length; j > 0; j--)
		{
			state_sequence[j - 1] = state_history[ state_sequence[j] ] [j]; 
		} 
		/* now figure out what input sequence corresponds to the state sequence in the optimal path */ 
    for (i = 0; i < channel_length; i++) 
	{
		*(decoder_output_matrix + m_outlength) = (byte)input[ state_sequence[i] ] [ state_sequence[i+1] ]; 
		m_outlength++;
	}
	*out_length = m_outlength;
	free(channel_output_matrix);

	for (i=0;i<TWOTOTHEM;i++)
	{
		delete state_history[i];
	}
	delete state_sequence;
	delete state_history;
}

int Viterbi::soft_metric(int data, int guess)
{ 
	
    return(abs(data - (guess * 7))); 
} 
 
/* this quantizer assumes that the mean channel_symbol value is +/- 1, 
   and translates it to an integer whose mean value is +/- 32 to address 
   the lookup table "quantizer_table". Overflow protection is included. 
*/ 
int Viterbi::soft_quant(byte channel_symbol)
 { 
    int x; 
	if(channel_symbol==0)
		x = -32;
	else
		x = 32;
    //x = (int) ( 32.0 * channel_symbol ); 
    if (x < -128) x = -128; 
    if (x > 127) x = 127; 
  
    return(quantizer_table[x + 128]); 
} 
/* this initializes a 3-bit soft-decision quantizer optimized for about 4 dB Eb/No. 
*/
void Viterbi::init_quantizer(void)
{ 
	
    int i; 
    for (i = -128; i < -31; i++) 
        quantizer_table[i + 128] = 7; 
    for (i = -31; i < -21; i++) 
        quantizer_table[i + 128] = 6; 
    for (i = -21; i < -11; i++) 
        quantizer_table[i + 128] = 5; 
    for (i = -11; i < 0; i++) 
        quantizer_table[i + 128] = 4; 
    for (i = 0; i < 11; i++) 
        quantizer_table[i + 128] = 3; 
    for (i = 11; i < 21; i++) 
        quantizer_table[i + 128] = 2; 
    for (i = 21; i < 31; i++) 
        quantizer_table[i + 128] = 1; 
    for (i = 31; i < 128; i++) 
        quantizer_table[i + 128] = 0; 
} 
/* this function calculates the next state of the convolutional encoder, given 
   the current state and the input data.  It also calculates the memory 
   contents of the convolutional encoder. */ 
int Viterbi::nxt_stat(int current_state, int input, int *memory_contents)
{ 
  
    int binary_state[Kc - 1];              /* binary value of current state */ 
    int next_state_binary[Kc - 1];         /* binary value of next state */ 
    int next_state;                       /* decimal value of next state */ 
    int i;                                /* loop variable */ 
 
    /* convert the decimal value of the current state number to binary */ 
    deci2bin(current_state, Kc - 1, binary_state); 
 
    /* given the input and current state number, compute the next state number */ 
    next_state_binary[0] = input; 
    for (i = 1; i < Kc - 1; i++) 
        next_state_binary[i] = binary_state[i - 1]; 
  
    /* convert the binary value of the next state number to decimal */ 
    next_state = bin2deci(next_state_binary, Kc - 1); 
 
    /* memory_contents are the inputs to the modulo-two adders in the encoder */ 
    memory_contents[0] = input; 
    for (i = 1; i < Kc; i++) 
        memory_contents[i] = binary_state[i - 1]; 
  
    return(next_state); 
} 
/* this function converts a decimal number to a binary number, stored 
   as a vector MSB first, having a specified number of bits with leading 
   zeroes as necessary */ 
void Viterbi::deci2bin(int d, int size, int *b)
{ 
    int i; 
  
    for(i = 0; i < size; i++) 
        b[i] = 0; 
  
    b[size - 1] = d & 0x01; 
  
    for (i = size - 2; i >= 0; i--)
	{ 
        d = d >> 1; 
        b[i] = d & 0x01; 
    } 
} 
/* this function converts a binary number having a specified 
   number of bits to the corresponding decimal number 
   with improvement contributed by Bryan Ewbank 2001.11.28 */ 
int Viterbi::bin2deci(int *b, int size)
{ 
    int i, d; 
  
    d = 0; 
  
    for (i = 0; i < size; i++) 
        d += b[i] << (size - i - 1); 
  
    return(d); 
} 