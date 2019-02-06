/*
Copyright (C) 2011 by the PSVN Research Group, University of Alberta
*/

#include <string.h>
#include "psvn.hpp"


static void resetAbstractions( const PSVN &psvn,
			       vector<vector<size_t> > &mapping,
			       vector<bool> &projection )
{
  size_t d, v;

  for( d = 0; d < psvn.numDomains(); ++d ) {
    mapping.push_back( vector<size_t>( psvn.domainSize( d ) ) );
    for( v = 0; v < psvn.domainSize( d ); ++v ) {
      mapping[ d ][ v ] = v;
    }
  }
  for( v = 0; v < psvn.numVariables(); ++v ) {
    projection[ v ] = false;
  }
}

static void printHelp()
{
  cout << "help or ?: print this message" << endl;
  cout << "quit or ctrl-d: save abstraction and quit" << endl;
  cout << "abort: quit without saving" << endl;
  cout << "print: print the current abstract game" << endl;
  cout << "domains: show the variable domains" << endl;
  cout << "variables: show the domains for each variable.  Variables which are removed" << endl;
  cout << "	by a projection are in parentheses" << endl;
  cout << "project <variable>: add or remove variable from projection abstraction" << endl;
  cout << "	for example, project 1 will add the first variable to the" << endl;
  cout << "	list of variables to remove by projection.  project -2 removes" << endl;
  cout << "	the second variable from the list (ie it remains in the abstraction)" << endl;
  cout << "map <domain> <value1> <value2>: map value1 in the named domain to value2" << endl;
}

static vector<string> getTokens( const string &line )
{
  size_t i;
  char c;
  vector<string> tokens;
  string tokenString;

  for( i = 0; i < line.size(); ++i ) {
    c = line.at( i );

    if( isspace( c ) ) {

      if( tokenString.length() > 0 ) {
	tokens.push_back( tokenString );
	tokenString.clear();
      }
    } else {

      tokenString.push_back( toupper( c ) );
    }
  }
  if( tokenString.length() > 0 ) {
    tokens.push_back( tokenString );
  }

  return tokens;
}


int main( int argc, char **argv )
{
  size_t d, v;
  string outFilename, abstFilename;
  ifstream is;
  ofstream os;
  string line;

  if( argc < 3 ) {
    cerr << "usage: abstraction PSVN_file output_prefix" << endl;
    exit( -1 );
  }

  outFilename = string( argv[ 2 ] );
  outFilename.append( ".psvn" );
  abstFilename = string( argv[ 2 ] );
  abstFilename.append( ".abst" );

  /* make sure we don't clobber our input file */
  if( outFilename.compare( argv[ 1 ] ) == 0
      || abstFilename.compare( argv[ 1 ] ) == 0 ) {
    cerr << "error: source and destination name must not be the same" << endl;
    exit( -1 );
  }

  /* read the game */
  is.open( argv[ 1 ] );
  if(!is) {
      cerr << "PSVN file " << argv[1] << " could not be opened." << endl;
      exit( -1 );
  }
  PSVN *psvn = PSVN::fromInput( is ); if( psvn == NULL ) { exit( -1 ); }

  vector<vector<size_t> > mapping;
  vector<bool> projection( psvn->numVariables() );
  resetAbstractions( *psvn, mapping, projection );

/* read, check, and record all the abstraction input */
  PSVN abstGame = *psvn;
  bool printGame = false;
  vector<string> tokens;

  printHelp();
  while( 1 ) {

    getline( cin, line );
    if( cin.fail() ) {
      break;
    }
    tokens = getTokens( line );
    if( tokens.size() == 0 ) {
      continue;
    }

    if( tokens[ 0 ].compare( "HELP" )  == 0
	|| tokens[ 0 ].compare( "?" ) == 0 ) {

      printHelp();
    } else if( tokens[ 0 ].compare( "QUIT" ) == 0 ) {

      break;
    } else if( tokens[ 0 ].compare( "ABORT" ) == 0 ) {

      return 0;
    } else if( tokens[ 0 ].compare( "PRINT" ) == 0 ) {

      printGame = true;
    } else if( tokens[ 0 ].compare( "DOMAINS" ) == 0 ) {

      for( d = 0; d < psvn->numDomains(); ++d ) {

	cout << psvn->domainName( d ) << endl;
	for( v = 0; v < psvn->domainSize( d ); ++v ) {

	  cout << " " << psvn->domainValueName( d, v )
	       << "->" << psvn->domainValueName( d, mapping[ d ][ v ] );
	}
        cout << endl;
      }
      cout << endl;
      continue;
    } else if( tokens[ 0 ].compare( "VARIABLES" ) == 0 ) {

      for( v = 0; v < psvn->numVariables(); ++v ) {

	if( v ) {
	  cout << " ";
	}
	if( projection[ v ] ) {

	  cout << "(" << psvn->domainName( psvn->variableDomain( v ) ) << ")";
	} else {
	  cout << psvn->domainName( psvn->variableDomain( v ) );
	}
      }
      cout << endl;
    } else if( tokens[ 0 ].compare( "PROJECT" ) == 0 ) {
      ssize_t p;

      if( tokens.size() < 2 ||
	  sscanf( tokens[ 1 ].c_str(), " %zd", &p ) < 1 ) {
	cerr << "could not get variable from " << line << endl;
	continue;
      }
      if( p == 0 ) {
	continue;
      }
      if( ( p > 0 && (size_t)p > psvn->numVariables() )
	  || ( p < 0 && (size_t)-p > psvn->numVariables() ) ) {
	cerr << "bad variable in " << line << endl;
	continue;
      }
      --p;

      if( p < 0 ) {
	projection[ -p ] = false;
      } else {
	projection[ p ] = true;
      }
    } else if( tokens[ 0 ].compare( "MAP" ) == 0 ) {
      size_t v2;

      if( tokens.size() < 4
	  || ( d = psvn->getDomain( tokens[ 1 ] ) ) == psvn->numDomains() ) {
	cerr << "could not get domain from " << line << endl;
	continue;
      }

      if( ( v = psvn->getDomainValue( d, tokens[ 2 ] ) )
	  == psvn->domainSize( d ) ) {
	cerr << "could not get value1 from " << line << endl;
	continue;
      }

      if( ( v2 = psvn->getDomainValue( d, tokens[ 3 ] ) )
	  == psvn->domainSize( d ) ) {
	cerr << "could not get value2 from " << line << endl;
	continue;
      }

      mapping[ d ][ v ] = v2;
    }



    /* All input has been read, now apply the abstractions to the given PSVN.
       We do domain abstraction first because the projection abstraction
       might add the new domain 1 = {0} */
    abstGame = *psvn;
    abstGame.domainAbstraction( mapping );
    abstGame.projectionAbstraction( projection );

    if( printGame ) {
      cout << abstGame;
      printGame = false;
    }
  }


  /* save the abstracted game */
  os.open( outFilename.c_str() );
  os << "# This file was created by the abstractor program." << endl ;
  os << "# The source PSVN file was: " << argv[1] << endl ;
  os << "# The abstraction used is stored in file: " << abstFilename << endl << endl ;
  os << abstGame;
  os.close();

  /* save the abstraction */
  os.open( abstFilename.c_str() );
  os << "abstraction {\n";
  for( d = 0; d < psvn->numDomains(); ++d ) {
      os << "  " << psvn->domainName(d) << " {";
      for( v = 0; v < psvn->domainSize( d ); ++v ) {
          os << " " << psvn->domainValues(d)[ mapping[ d ][ v ] ];
      }
      os << " }\n";
  }
  os << "  projection {";
  for( v = 0; v < psvn->numVariables(); ++v ) {
      if( projection[ v ] ) {
          os << " P";
      } else {
          os << " K";
      }
  }
  os << " }\n}\n";
  os.close();
}
