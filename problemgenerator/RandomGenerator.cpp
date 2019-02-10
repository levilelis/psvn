#include <iostream>
#include <time.h>
#include <math.h>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <sstream>
//#include <stdlib.h>
#include "../randomc/randomc.h"
#include "../randomc/mersenne.cpp"

using namespace::std;

/* generate a state using backwards moves from the goal
   returns non-zero on success, 0 on failure

   only uses the first goal state - if there are more goal states, you
   may want to change this to count them and then select a random goal

   NOTE THAT THIS MAY GENERATE SYNTACTICALLY CORRECT BUT INVALID
   STATES IF NOT ALL STATES ARE VALID AND AT LEAST ONE GAME RULE
   ASSIGNS A CONSTANT TO A STATE VARIABLE WHICH IS EITHER A LABEL OR
   UNSPECIFIED ON THE LEFT HAND SIDE */

int random_state( state_t *state, CRandomMersenne * RandGen )
{
  int i, max_i, num_moves, j;
  func_ptr iter;
  int rules[ bw_max_children ];
  state_t child, *s, *c, *t;

  s = state;
  c = &child;

  random_goal_state( state );
  max_i = 100;

   for( i = 0; i < max_i; ++i ) {

    num_moves = 0;
    init_bwd_iter( iter );
    while( ( rules[ num_moves ] = next_bwd_iter( iter, s ) ) >= 0 ) {
      ++num_moves;
    }

    if( num_moves == 0 ) {
      printf( "no moves at state " );
      print_state( stdout, s );
      printf( "\n" );
      if( i ) {
        printf( "parent " );
        print_state( stdout, c );
        printf( "\n" );
      }
      exit( -1 );
    }

    j = RandGen->IRandom(0,num_moves - 1);
    //cout << j << endl;

    apply_bwd_rule( rules[ j ], s, c );
    t = s;
    s = c;
    c = t;
  }

  if( s != state ) {
    copy_state( state, s );
  }

  return 0;
}

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		cout << "Invalid arguments: <number of samples> <output file> <single file? (0 or 1)>" << endl;
		exit(-1);
	}

	int numberSamples = atoi( argv[1] );
	string outputFile( argv[2] );
	int singleFile = atoi( argv[3] );

	CRandomMersenne * RanGen = new CRandomMersenne( ( unsigned )time( NULL ) );

	state_t state;

	if(singleFile == 1)
	{
		for( int i = 0; i < numberSamples; i++ )
		{
			FILE * file;

			ostringstream ss;
			ss << i + 1;
			ss << ".pro";
			string output = outputFile;

			output.append(ss.str());
			file = fopen( output.c_str(), "w" );

			random_state( &state, RanGen );
			print_state( file, &state );
			fprintf(file, "\n");

			fclose( file );
		}
	}
	else
	{
		FILE * file;

		ostringstream ss;
		ss << ".pro";
		string output = outputFile;

		output.append(ss.str());
		file = fopen( output.c_str(), "w" );

		for( int i = 0; i < numberSamples; i++ )
		{
			random_state( &state, RanGen );
			print_state( file, &state );
			fprintf(file, "\n");
		}

		fclose( file );
	}

	delete RanGen;

	return 0;
}
