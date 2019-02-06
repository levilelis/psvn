/*
Copyright (C) 2011 by the PSVN Research Group, University of Alberta
*/

#include <limits.h>
#include <string.h>

/* copy a state */
#define copy_state(dest_ptr,src_ptr) memcpy(dest_ptr,src_ptr,sizeof(var_t)*NUMVARS)

/* compare two state pointers
   returns 0 if equal, non-zero otherwise */
#define compare_states(a,b) memcmp(a,b,sizeof(var_t)*NUMVARS)

/* returns cost of cheapest forward rule, or MAX_INT if no rule applies */
static int cost_of_cheapest_applicable_fwd_rule(const state_t* state)
{
    int rule_used;
    int mincost = INT_MAX;
    func_ptr iter;
    init_fwd_iter(iter);
    while ((rule_used = next_fwd_iter(iter, state)) >= 0) {
        int curcost = fwd_rule_costs[rule_used];
        if (curcost < mincost)
            mincost = curcost;            
    }
    return mincost;
}

/* returns cost of cheapest backward rule, or MAX_INT if no rule applies */
static int cost_of_cheapest_applicable_bwd_rule(const state_t* state)
{
    int rule_used;
    int mincost = INT_MAX;
    func_ptr iter;
    init_bwd_iter(iter);
    while ((rule_used = next_bwd_iter(iter, state)) >= 0) {
        int curcost = bwd_rule_costs[rule_used];
        if (curcost < mincost)
            mincost = curcost;            
    }
    return mincost;
}

/* print a state to a file
   returns number of characters on success, -1 on failure */
static ssize_t print_state( FILE *file, const state_t *state )
{
  size_t len, t; int i;
  for( len = 0, i = 0; i < NUMVARS; ++i ) {
    t = fprintf( file, "%s ", var_domain_names[ i ][ state->vars[ i ] ] );
    if( t < 0 ) { return -1; }
    len += t;
  }
  return len;
}

/* print a state to a string
   returns number of characters on success, -1 on failure */
static ssize_t sprint_state( char *string,const size_t max_len,const state_t *state )
{
  size_t len, t; int i;
  for( len = 0, i = 0; i < NUMVARS; ++i ) {
    t = snprintf( &string[ len ], max_len - len, "%s ",
		  var_domain_names[ i ][ state->vars[ i ] ] );
    if( t < 0 ) { return -1; }
    len += t;
  }
  if( len >= max_len ) { return -1; } else { string[ len ] = 0; }
  return len;
}

/* read a state from a string
   returns number of characters consumed on success, -1 on failure
   NOTE: the part of the string representing the state must be
   followed either by whitespace (which will be consumed) or
   the end of string marker */
static ssize_t read_state( const char *string, state_t *state )
{
  size_t len, t; int i; var_t j;
  for( len = 0, i = 0; i < NUMVARS; ++i ) {
    for( j = 0; j < domain_sizes[ var_domains[ i ] ]; ++j ) {
      t = strlen( var_domain_names[ i ][ j ] );
      if( !strncasecmp( var_domain_names[ i ][ j ], &string[ len ], t ) ) {
	if( isspace( string[ len + t ] ) ) {
	  state->vars[ i ] = j;
	  len += t + 1;
	  break;
	} else if( string[ len + t ] == 0 ) {
	  state->vars[ i ] = j;
	  len += t;
	  break;
	}
      }
    }
    if( j >= domain_sizes[ var_domains[ i ] ] ) { return -1; }
  }
  return len;
}

/* dump a raw state to a file
   returns 1 on success, 0 on failure */
#define dump_state( file, state_ptr ) fwrite(state_ptr,sizeof(var_t)*NUMVARS,1,file)

/* load a raw state from a file
   returns 1 on success, 0 on failure */
#define load_state( file, state_ptr ) fread(state_ptr,sizeof(var_t)*NUMVARS,1,file)


typedef struct {
  state_t state;
  int value;
} state_map_entry_t;

typedef struct {
  state_map_entry_t *entries;
  int64_t avail_entries;
  int64_t max_entry;
} state_map_t;

/* create a map of states to values */
static state_map_t *new_state_map()
{
  state_map_t *map;
  int64_t i;
  map = (state_map_t *)malloc( sizeof( *map ) );
  assert( map != 0 );
  map->max_entry = 1023;
  map->avail_entries = (float)map->max_entry * 0.75;
  map->entries = (state_map_entry_t *)malloc( sizeof( map->entries[ 0 ] )
			 * ( map->max_entry + 1 ) );
  assert( map->entries != 0 );
  for( i = 0; i <= map->max_entry; ++i ) {
    map->entries[ i ].state.vars[ 0 ] = -1;
  }
  return map;
}

/* destroy a state map, freeing all associated memory */
static void destroy_state_map( state_map_t *map )
{
  free( map->entries );
  free( map );
}

/*
-------------------------------------------------------------------------------
lookup3.c, by Bob Jenkins, May 2006, Public Domain.

These are functions for producing 32-bit hashes for hash table lookup.
hashword(), hashlittle(), hashlittle2(), hashbig(), mix(), and final() 
are externally useful functions.  Routines to test the hash are included 
if SELF_TEST is defined.  You can use this free for any purpose.  It's in
the public domain.  It has no warranty.

You probably want to use hashlittle().  hashlittle() and hashbig()
hash byte arrays.  hashlittle() is is faster than hashbig() on
little-endian machines.  Intel and AMD are little-endian machines.
On second thought, you probably want hashlittle2(), which is identical to
hashlittle() except it returns two 32-bit hashes for the price of one.  
You could implement hashbig2() if you wanted but I haven't bothered here.

If you want to find a hash of, say, exactly 7 integers, do
  a = i1;  b = i2;  c = i3;
  mix(a,b,c);
  a += i4; b += i5; c += i6;
  mix(a,b,c);
  a += i7;
  final(a,b,c);
then use c as the hash value.  If you have a variable length array of
4-byte integers to hash, use hashword().  If you have a byte array (like
a character string), use hashlittle().  If you have several byte arrays, or
a mix of things, see the comments above hashlittle().  

Why is this so big?  I read 12 bytes at a time into 3 4-byte integers, 
then mix those integers.  This is fast (you can do a lot more thorough
mixing with 12*3 instructions on 3 integers than you can with 3 instructions
on 1 byte), but shoehorning those bytes into integers efficiently is messy.
-------------------------------------------------------------------------------
*/
#include <sys/param.h>  /* attempt to define endianness */
#ifdef linux
# include <endian.h>    /* attempt to define endianness */
#endif

/*
 * My best guess at if you are big-endian or little-endian.  This may
 * need adjustment.
 */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && \
     __BYTE_ORDER == __LITTLE_ENDIAN) || \
    (defined(i386) || defined(__i386__) || defined(__i486__) || \
     defined(__i586__) || defined(__i686__) || defined(vax) || defined(MIPSEL))
# define HASH_LITTLE_ENDIAN 1
# define HASH_BIG_ENDIAN 0
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && \
       __BYTE_ORDER == __BIG_ENDIAN) || \
      (defined(sparc) || defined(POWERPC) || defined(mc68000) || defined(sel))
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 1
#else
# define HASH_LITTLE_ENDIAN 0
# define HASH_BIG_ENDIAN 0
#endif

#define rot(x,k) (((x)<<(k)) ^ ((x)>>(32-(k))))

/*
-------------------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.

This is reversible, so any information in (a,b,c) before mix() is
still in (a,b,c) after mix().

If four pairs of (a,b,c) inputs are run through mix(), or through
mix() in reverse, there are at least 32 bits of the output that
are sometimes the same for one pair and different for another pair.
This was tested for:
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

Some k values for my "a-=c; a^=rot(c,k); c+=b;" arrangement that
satisfy this are
    4  6  8 16 19  4
    9 15  3 18 27 15
   14  9  3  7 17  3
Well, "9 15 3 18 27 15" didn't quite get 32 bits diffing
for "differ" defined as + with a one-bit base and a two-bit delta.  I
used http://burtleburtle.net/bob/hash/avalanche.html to choose 
the operations, constants, and arrangements of the variables.

This does not achieve avalanche.  There are input bits of (a,b,c)
that fail to affect some output bits of (a,b,c), especially of a.  The
most thoroughly mixed value is c, but it doesn't really even achieve
avalanche in c.

This allows some parallelism.  Read-after-writes are good at doubling
the number of bits affected, so the goal of mixing pulls in the opposite
direction as the goal of parallelism.  I did what I could.  Rotates
seem to cost as much as shifts on every machine I could lay my hands
on, and rotates are much kinder to the top and bottom bits, so I used
rotates.
-------------------------------------------------------------------------------
*/
#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

/*
-------------------------------------------------------------------------------
final -- final mixing of 3 32-bit values (a,b,c) into c

Pairs of (a,b,c) values differing in only a few bits will usually
produce values of c that look totally different.  This was tested for
* pairs that differed by one bit, by two bits, in any combination
  of top bits of (a,b,c), or in any combination of bottom bits of
  (a,b,c).
* "differ" is defined as +, -, ^, or ~^.  For + and -, I transformed
  the output delta to a Gray code (a^(a>>1)) so a string of 1's (as
  is commonly produced by subtraction) look like a single 1-bit
  difference.
* the base values were pseudorandom, all zero but one bit set, or 
  all zero plus a counter that starts at zero.

These constants passed:
 14 11 25 16 4 14 24
 12 14 25 16 4 14 24
and these came close:
  4  8 15 26 3 22 24
 10  8 15 26 3 22 24
 11  8 15 26 3 22 24
-------------------------------------------------------------------------------
*/
#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

/*
 * hashlittle2: return 2 32-bit hash values
 *
 * This is identical to hashlittle(), except it returns two 32-bit hash
 * values instead of just one.  This is good enough for hash table
 * lookup with 2^^64 buckets, or if you want a second hash if you're not
 * happy with the first, or if you want a probably-unique 64-bit ID for
 * the key.  *pc is better mixed than *pb, so use *pc first.  If you want
 * a 64-bit value do something like "*pc + (((uint64_t)*pb)<<32)".
 */
static void hashlittle2( 
  const void *key,       /* the key to hash */
  size_t      length,    /* length of the key */
  uint32_t   *pc,        /* IN: primary initval, OUT: primary hash */
  uint32_t   *pb)        /* IN: secondary initval, OUT: secondary hash */
{
  uint32_t a,b,c;                                          /* internal state */
  union { const void *ptr; size_t i; } u;     /* needed for Mac Powerbook G4 */

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + ((uint32_t)length) + *pc;
  c += *pb;

  u.ptr = key;
  if (HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0)) {
    const uint32_t *k = (const uint32_t *)key;         /* read 32-bit chunks */
#ifdef VALGRIND
    const uint8_t  *k8;
#endif

    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      b += k[1];
      c += k[2];
      mix(a,b,c);
      length -= 12;
      k += 3;
    }

    /*----------------------------- handle the last (probably partial) block */
    /* 
     * "k[2]&0xffffff" actually reads beyond the end of the string, but
     * then masks off the part it's not allowed to read.  Because the
     * string is aligned, the masked-off tail is in the same word as the
     * rest of the string.  Every machine with memory protection I've seen
     * does it on word boundaries, so is OK with this.  But VALGRIND will
     * still catch it and complain.  The masking trick does make the hash
     * noticably faster for short strings (like English words).
     */
#ifndef VALGRIND

    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
    case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
    case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
    case 6 : b+=k[1]&0xffff; a+=k[0]; break;
    case 5 : b+=k[1]&0xff; a+=k[0]; break;
    case 4 : a+=k[0]; break;
    case 3 : a+=k[0]&0xffffff; break;
    case 2 : a+=k[0]&0xffff; break;
    case 1 : a+=k[0]&0xff; break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

#else /* make valgrind happy */

    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
    case 11: c+=((uint32_t)k8[10])<<16;  /* fall through */
    case 10: c+=((uint32_t)k8[9])<<8;    /* fall through */
    case 9 : c+=k8[8];                   /* fall through */
    case 8 : b+=k[1]; a+=k[0]; break;
    case 7 : b+=((uint32_t)k8[6])<<16;   /* fall through */
    case 6 : b+=((uint32_t)k8[5])<<8;    /* fall through */
    case 5 : b+=k8[4];                   /* fall through */
    case 4 : a+=k[0]; break;
    case 3 : a+=((uint32_t)k8[2])<<16;   /* fall through */
    case 2 : a+=((uint32_t)k8[1])<<8;    /* fall through */
    case 1 : a+=k8[0]; break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

#endif /* !valgrind */

  } else if (HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0)) {
    const uint16_t *k = (const uint16_t *)key;         /* read 16-bit chunks */
    const uint8_t  *k8;

    /*--------------- all but last block: aligned reads and different mixing */
    while (length > 12)
    {
      a += k[0] + (((uint32_t)k[1])<<16);
      b += k[2] + (((uint32_t)k[3])<<16);
      c += k[4] + (((uint32_t)k[5])<<16);
      mix(a,b,c);
      length -= 12;
      k += 6;
    }

    /*----------------------------- handle the last (probably partial) block */
    k8 = (const uint8_t *)k;
    switch(length)
    {
    case 12: c+=k[4]+(((uint32_t)k[5])<<16);
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 11: c+=((uint32_t)k8[10])<<16;     /* fall through */
    case 10: c+=k[4];
             b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 9 : c+=k8[8];                      /* fall through */
    case 8 : b+=k[2]+(((uint32_t)k[3])<<16);
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 7 : b+=((uint32_t)k8[6])<<16;      /* fall through */
    case 6 : b+=k[2];
             a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 5 : b+=k8[4];                      /* fall through */
    case 4 : a+=k[0]+(((uint32_t)k[1])<<16);
             break;
    case 3 : a+=((uint32_t)k8[2])<<16;      /* fall through */
    case 2 : a+=k[0];
             break;
    case 1 : a+=k8[0];
             break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }

  } else {                        /* need to read the key one byte at a time */
    const uint8_t *k = (const uint8_t *)key;

    /*--------------- all but the last block: affect some 32 bits of (a,b,c) */
    while (length > 12)
    {
      a += k[0];
      a += ((uint32_t)k[1])<<8;
      a += ((uint32_t)k[2])<<16;
      a += ((uint32_t)k[3])<<24;
      b += k[4];
      b += ((uint32_t)k[5])<<8;
      b += ((uint32_t)k[6])<<16;
      b += ((uint32_t)k[7])<<24;
      c += k[8];
      c += ((uint32_t)k[9])<<8;
      c += ((uint32_t)k[10])<<16;
      c += ((uint32_t)k[11])<<24;
      mix(a,b,c);
      length -= 12;
      k += 12;
    }

    /*-------------------------------- last block: affect all 32 bits of (c) */
    switch(length)                   /* all the case statements fall through */
    {
    case 12: c+=((uint32_t)k[11])<<24;
    case 11: c+=((uint32_t)k[10])<<16;
    case 10: c+=((uint32_t)k[9])<<8;
    case 9 : c+=k[8];
    case 8 : b+=((uint32_t)k[7])<<24;
    case 7 : b+=((uint32_t)k[6])<<16;
    case 6 : b+=((uint32_t)k[5])<<8;
    case 5 : b+=k[4];
    case 4 : a+=((uint32_t)k[3])<<24;
    case 3 : a+=((uint32_t)k[2])<<16;
    case 2 : a+=((uint32_t)k[1])<<8;
    case 1 : a+=k[0];
             break;
    case 0 : *pc=c; *pb=b; return;  /* zero length strings require no mixing */
    }
  }

  final(a,b,c);
  *pc=c; *pb=b; return;             /* zero length strings require no mixing */
}
/* ----------------------------------------------------------------------
   end lookup3
   ---------------------------------------------------------------------- */

static uint64_t hash_state( const state_t *state )
{
  uint32_t a, b;

  a = 0; b = 0; hashlittle2( state, sizeof( var_t ) * NUMVARS, &a, &b );
  return ( (uint64_t)a << 32 ) | b;
}

static uint64_t hash_state_history( const state_t *state, const int history )
{
  uint32_t a, b;

  a = b = history; hashlittle2( state, sizeof( var_t ) * NUMVARS, &a, &b );
  return ( (uint64_t)a << 32 ) | b;
}

static int64_t state_map_hash_state( const state_map_t *map,
				     const state_t *state )
{
  uint64_t index, mult;

  index = hash_state( state ) & map->max_entry;
  mult = 1;
  while( map->entries[ index ].state.vars[ 0 ] >= 0 ) {
    if( !compare_states( state, &map->entries[ index ].state ) ) {
      break;
    }
    index = ( index + mult ) & map->max_entry;
    ++mult;
  }
  return index;
}

/* add state->value to the map.
   Replaces previous mapping if state is already in the map. */
static void state_map_add( state_map_t *map, const state_t *state, const int value )
{
  int64_t idx;
  if( map->avail_entries == 0 ) {
    int64_t i;
    state_map_entry_t *old_entries;
    i = map->max_entry;
    map->max_entry = map->max_entry * 2 + 1;
    map->avail_entries = (float)map->max_entry * 0.75;
    old_entries = map->entries;
    map->entries = (state_map_entry_t *)malloc( sizeof( map->entries[ 0 ] )
						* ( map->max_entry + 1 ) );
    assert( map->entries != 0 );
    for( idx = 0; idx <= map->max_entry; ++idx ) {
      map->entries[ idx ].state.vars[ 0 ] = -1;
    }
    while( 1 ) {
      if( old_entries[ i ].state.vars[ 0 ] >= 0 ) {
	state_map_add( map, &old_entries[ i ].state, old_entries[ i ].value );
      }
      if( i == 0 ) { break; }
      --i;
    }
    free( old_entries );
  }
  idx = state_map_hash_state( map, state );
  if( map->entries[ idx ].state.vars[ 0 ] < 0 ) {
    copy_state( &map->entries[ idx ].state, state );
    --map->avail_entries;
  }
  map->entries[ idx ].value = value;
}

/* returns NULL if state is not in map
   returns a pointer to the value if state is in the map */
static int *state_map_get( const state_map_t *map, const state_t *state )
{
  uint64_t idx = state_map_hash_state( map, state );
  if( map->entries[ idx ].state.vars[ 0 ] < 0 ) {
    return 0;
  }
  return &map->entries[ idx ].value;
}

static void write_state_map( FILE *file, const state_map_t *map )
{
  size_t written;
  written = fwrite( &map->max_entry, sizeof( map->max_entry ), 1, file );
  assert( written == 1 );
  written = fwrite( &map->avail_entries,
		    sizeof( map->avail_entries ), 1, file );
  assert( written == 1 );
  written = fwrite( map->entries, sizeof( map->entries[ 0 ] ),
		    map->max_entry + 1, file );
  assert( written == (size_t)map->max_entry + 1 );
}

static state_map_t *read_state_map( FILE *file )
{
  int64_t max_entry;
  state_map_t *map;
  size_t read_in;
  read_in = fread( &max_entry, sizeof( max_entry ), 1, file );
  assert( read_in == 1 );
  map = (state_map_t *)malloc( sizeof( *map ) );
  assert( map != NULL );
  map->max_entry = max_entry;
  map->entries = (state_map_entry_t *)
    malloc( sizeof( map->entries[ 0 ] ) * ( map->max_entry + 1 ) );
  assert( map->entries != NULL );
  read_in = fread( &map->avail_entries, sizeof( map->avail_entries ), 1, file );
  assert( read_in == 1 );
  read_in = fread( map->entries, sizeof( map->entries[ 0 ] ),
		   map->max_entry + 1, file );
  assert( read_in == (size_t)map->max_entry + 1 );
  return map;
}

static void read_dyn_table(const int* data, var_test_t*** table)
{
    int i, j, size;
    size = *data++;
    *table = (var_test_t**) malloc (sizeof(var_test_t*) * size);

    /** NOTE: this is dumb! no need to copy it, just point into data */
    for (i = 0; i < size; ++i) {
        int type = *data++;
        int var = *data++;
        int other = *data++;
        int rule = *data++;
        int num_edges = 0;
        if (type == 1 || type == 2)
            num_edges = 2;
        else if (type == 3)
            num_edges = domain_sizes[ var_domains[ var ] ];

        var_test_t* entry = (var_test_t*) 
            malloc (sizeof(var_test_t) + num_edges * sizeof(int));

        entry->type = type;
        entry->var = var;
        entry->other = other;
        entry->rule = rule;
        for (j = 0; j < num_edges; ++j)
            entry->edges[j] = *data++;
        (*table)[i] = entry;
    }
}

static void init_dyn_abstractions()
{
    read_dyn_table(fwd_var_test_table_data, &fwd_var_test_table);
    read_dyn_table(bwd_var_test_table_data, &bwd_var_test_table);
}

static int next_dyn_iter(const state_t* state, dyn_iter_t* iter,
                         const abstraction_t* abst,
                         const int* abst_label_sets,
                         var_test_t const* const* var_test_table)
{
    int rule = -1;
    while (1) {

        /* try getting node off stack */
        while(iter->id == -1) {

            if (iter->num == 0)  /* stack empty: finished! */ 
                return -1;

            iter->id = iter->id_stack[--iter->num];
        }

        //fprintf(stderr, "iter->id = %d\n", iter->id);
        //fprintf(stderr, "%p\n", var_test_table);

        const var_test_t* test = var_test_table[iter->id];
        switch (test->type) {
        case 0: /* no checks; just a rule */
            iter->id = test->other;
            rule = test->var;
            break;
            
        case 1: /* equality check */
            {
                /* find representative of label under this abstraction */
                const int other 
                    = abst_label_sets[test->rule*NUMVARS + test->var];

                if (abst->project_away_var[test->var] || other == test->var)
                {
                    /* Take first branch; add second branch to stack */
                    iter->id = test->edges[0];
                    iter->id_stack[iter->num++] = test->edges[1];
                    assert(iter->num < 256);

                } else {
                    const int branch = state->vars[test->var] 
                                    == state->vars[other];
                    iter->id = test->edges[ branch ];
                }
            }
            break;
            
        case 2: /* binary-valued constant test */
            {
                int branch = 0;
                if (abst->project_away_var[test->var]) 
                    branch = -1; /* take both! */
                else {
                    const int v = state->vars[test->var];
                    const int d = var_domains[test->var];
                    const int t = test->other;
                    if (v == t) {
                        if (abst->value_map[d][t] == t)
                            if (abst->mapped_in[d][t].size == 1)
                                branch = 1;
                            else
                                branch = -1; /* take both! */
                        else
                            branch = 0;
                    } else {
                        if (abst->value_map[d][t] == v)
                            branch = -1; /* take both! */
                        else
                            branch = 0;
                    }
                }
                if (branch >= 0) {
                    iter->id = test->edges[ branch ];
                } else {
                    /* take first; add second to stack */
                    iter->id = test->edges[0];
                    iter->id_stack[iter->num++] = test->edges[1];
                    assert(iter->num < 256);
                }
            }
            break;

        case 3: /* multi-valued constant test */
            {
                if (abst->project_away_var[test->var]) {
                    /* Visit all possible values if variable projected away */
                    int i;
                    const int d = var_domains[test->var];
                    iter->id = test->edges[0];
                    assert(iter->num + domain_sizes[d] - 1 < 256);
                    for (i = domain_sizes[d] - 1; i >= 1; --i) {
                        /* FIXME: do not push duplicate ids! */
                        iter->id_stack[iter->num++] = test->edges[ i ];
                    }
                }
                else 
                {   /* Visit all values that map into current value */
                    int i;
                    const int v = state->vars[test->var];
                    const int d = var_domains[test->var];
                    assert(abst->mapped_in[d][v].size > 0);
                    assert(iter->num + abst->mapped_in[d][v].size - 1 < 256);
                    for (i = abst->mapped_in[d][v].size - 1; i >= 1; --i) {
                        /* FIXME: do not push duplicate ids! */
                        iter->id_stack[iter->num++] 
                            = test->edges[ abst->mapped_in[d][v].v[i] ];
                    }
                    iter->id = test->edges[ abst->mapped_in[d][v].v[0] ];
                }
            }
            break;
        }
        if (rule != -1)
            return rule;
    }
    return -1;
}

static abstraction_t* allocate_abstraction()
{
    int i;
    int64_t s;
    abstraction_t* abst = (abstraction_t *)malloc( sizeof( *abst ) );
    if( abst == NULL )
        return NULL;
    
    for( s = 0, i = 0; i < NUMDOMAINS; ++i ) {
        s += domain_sizes[ i ];
    }
    abst->value_map[ 0 ]
        = (var_t * )malloc( sizeof( abst->value_map[ 0 ][ 0 ] ) * s );
    if( abst->value_map[ 0 ] == NULL ) {
        free( abst );
        return NULL;
    }
    abst->mapped_in[ 0 ] 
        = (abst_array_t*)malloc(sizeof(abst->mapped_in[0][0]) * s);
    if (abst->mapped_in[ 0 ] == NULL) {
        free(abst->value_map[0]);
        free(abst);
        return NULL;
    }
    for( s = domain_sizes[ 0 ], i = 1;
         i < NUMDOMAINS;
         s += domain_sizes[ i ], ++i ) 
    {
        abst->value_map[ i ] = &abst->value_map[ 0 ][ s ];
        abst->mapped_in[ i ] = &abst->mapped_in[ 0 ][ s ];
    }

    for (i = 0; i < NUMDOMAINS; ++i){
        abst->mapped_in[ i ][ 0 ].v 
            = (var_t*) malloc (sizeof(var_t) * domain_sizes[i]);
    }

    return abst;
}

static void destroy_abstraction( abstraction_t *abst )
{
    int i;
    for (i = 0; i < NUMDOMAINS; ++i)
        free ( abst->mapped_in[i][0].v );
    free( abst->mapped_in[ 0 ] );
    free( abst->value_map[ 0 ] );
    free( abst );
}

/* Fills in an abstraction's mapped_in array.
   Required for use in a dyanmic abstraction setting. Overwrites old
   mapped_in array. */
static void abstraction_compute_mapped_in(abstraction_t* abst)
{
    int i, j, k, n;
    int found[128];
    size_t size;
    for( i = 0; i < NUMDOMAINS; ++i ) {
        var_t* in = abst->mapped_in[i][0].v;
        for (j = 0; j < domain_sizes[i]; ++j) {
            abst->mapped_in[i][j].size = 0;
            abst->mapped_in[i][j].v = in;
            for (k = 0; k < domain_sizes[i]; ++k) {
                if (abst->value_map[i][k] == j) {
                    abst->mapped_in[i][j].size++;
                    *in++ = k;
                }
            }
        }
    }

    /* Compute the representative for variable equality comparisions.
       Suppose the LHS of a rule looks like "- A A A". The compiler
       will add the tests "var[2] == var[1]" and "var[3] ==
       var[1]". But what happens if var[1] is projected away? We need
       to compute the new representative of 'A' (which the compiler
       set to var[1] initially), by finding another A that isn't
       projected away, and use it for the comparison tests. */
    size = NUMVARS * NUM_FWD_RULES * sizeof(int);
    abst->fwd_rule_label_sets = (int*) malloc (size);
    memcpy(abst->fwd_rule_label_sets, fwd_rule_label_sets, size);
    for (i = 0; i < NUM_FWD_RULES; ++i) {
        for (j = 0; j < NUMVARS; ++j) {
            if (abst->project_away_var[j]) {
                n = 0;
                for (k = j + 1; k < NUMVARS; ++k)
                    if (!abst->project_away_var[k] 
                        && fwd_rule_label_sets[i*NUMVARS + k] == j)
                        found[n++] = k;
                /* Map others to new representative. */
                if (n > 0) {
                    abst->fwd_rule_label_sets[i*NUMVARS + j] = found[0];
                    for (k = 0; k < n; ++k)
                        abst->fwd_rule_label_sets[i*NUMVARS + found[k]] = found[0];
                }
            }
        }
    }

    /* Do the same for the backwards rules */
    size = NUMVARS * NUM_BWD_RULES * sizeof(int);
    abst->bwd_rule_label_sets = (int*) malloc (size);
    memcpy(abst->bwd_rule_label_sets, bwd_rule_label_sets, size);
    for (i = 0; i < NUM_BWD_RULES; ++i) {
        for (j = 0; j < NUMVARS; ++j) {
            if (abst->project_away_var[j]) {
                n = 0;
                for (k = j + 1; k < NUMVARS; ++k)
                    if (!abst->project_away_var[k] 
                        && bwd_rule_label_sets[i*NUMVARS + k] == j)
                        found[n++] = k;
                if (n > 0) {
                    abst->bwd_rule_label_sets[i*NUMVARS + j] = found[0];
                    for (k = 0; k < n; ++k)
                        abst->bwd_rule_label_sets[i*NUMVARS + found[k]] = found[0];
                }
            }
        }
    }
}

static abstraction_t* create_identity_abstraction()
{
    int i, j;
    abstraction_t* abst = allocate_abstraction();
    if (abst == NULL)
        return NULL;

    for( i = 0; i < NUMDOMAINS; ++i )
        for( j = 0; j < domain_sizes[ i ]; ++j )
            abst->value_map[ i ][ j ] = j;
    abstraction_compute_mapped_in( abst );

    for( i = 0; i < NUMVARS; ++i )
        abst->project_away_var[ i ] = 0;

    return abst;
}

/* Reads abstraction from stream between closing curly braces.
   Assumes abstraction starts as the identity map. Only domains
   you want to change need to specified. */
static abstraction_t *read_abstraction_from_stream( FILE* stream )
{
    int i, k;
    var_t j;
    char token[1024];
    abstraction_t *abst = create_identity_abstraction();
    if (!abst)
        return NULL;

    if (!fscanf(stream, " %s", token) || token[0] != '{') {
        fprintf(stderr, "Missing opening '{'!\n");
        destroy_abstraction( abst );
        return NULL;
    }

    while (!feof(stream)) {
        if (!fscanf(stream, " %s ", token)) {
            fprintf(stderr, "Expected more input!\n");
            destroy_abstraction( abst );
            return NULL;
        }
        if (token[0] == '}')
            break;
        else if (!strcasecmp(token, "projection")) 
        {
            if (!fscanf(stream, " %s", token) || token[0] != '{') {
                fprintf(stderr, "Missing opening '{' for projection.\n");
                destroy_abstraction( abst );
                return NULL;
            }

            /* set the projection mapping */
            for( i = 0; i < NUMVARS; ++i ) {
                if(!fscanf(stream, " %s", token ) ) {
                    destroy_abstraction( abst );
                    fclose(stream);
                    return NULL;
                }
                if( token[0] == 'p' || token[0] == 'P' ) {
                    abst->project_away_var[ i ] = 1;
                } else if (token[0] == 'k' || token[0] == 'K') {
                    abst->project_away_var[ i ] = 0;
                } else {
                    fprintf(stderr, "Bad projection value: '%s'\n", token);
                    destroy_abstraction( abst );
                    return NULL;
                }
            }
            if (!fscanf(stream, " %s", token) || token[0] != '}') {
                fprintf(stderr, "Missing closing '}' after projection\n");
                destroy_abstraction( abst );
                return NULL;
            }

        } else {

            /* find domain */
            for (i = 0; i < NUMDOMAINS; ++i) {
                if (!strcasecmp(token, name_of_domain[i]))
                    break;
            }
            if (i == NUMDOMAINS) {
                fprintf(stderr, "Bad domain name! '%s'\n", token);
                destroy_abstraction( abst );
                return NULL;
            }

            if (!fscanf(stream, " %s", token) || token[0] != '{') {
                fprintf(stderr, "Missing opening '{' for domain mapping.\n");
                destroy_abstraction( abst );
                return NULL;
            }

            /* read domain mapping */
            for (j = 0; j < domain_sizes[ i ]; ++j) {
                if (!fscanf(stream, " %s", token)) {
                    fprintf(stderr, "Missing domain value!\n");
                    destroy_abstraction( abst );
                    return NULL;
                }
                for (k = 0; k < domain_sizes[i]; ++k) {
                    if (!strcasecmp(domain_to_domain_names[i][k], token))
                        break;
                }
                if (k == domain_sizes[i]) {
                    fprintf(stderr, "Bad domain value! '%s'\n", token);
                    destroy_abstraction( abst );
                    return NULL;
                }
                abst->value_map[i][j] = k;
            }

            if (!fscanf(stream, " %s", token) || token[0] != '}') {
                fprintf(stderr, "Missing closing '}' after domain mapping\n");
                destroy_abstraction( abst );
                return NULL;
            }
        }
    }

    return abst;
}

/* Reads an abstraction from a file.
   Returns the abstraction on success, or NULL on failure */
static abstraction_t *read_abstraction_from_file( const char *filename )
{
    char token[1024];
    FILE *file;
    file = fopen( filename, "r" );
    if( file == NULL )
        return NULL;

    if (!fscanf(file, "%s", token) || strcasecmp(token, "abstraction") ) {
        fprintf(stderr, "Missing opening \"abstraction\" token!\n");
        return NULL;
    }
    abstraction_t* abst = read_abstraction_from_stream( file );
    fclose( file );
    return abst;
}

static void print_abstraction( const abstraction_t* abst )
{
    int i, j;
    printf("abstraction {\n");
    for( i = 0; i < NUMDOMAINS; ++i ) {
        printf("  %s {", name_of_domain[ i ]);
        for( j = 0; j < domain_sizes[ i ]; ++j ) {
            if (j) printf(", ");
            printf("%s", domain_to_domain_names[i][ abst->value_map[i][j] ]);
        }
        printf(" }  \n");
    }
    printf("  projection {");
    for (i = 0; i < NUMVARS; ++i) {
        printf(" %c", (abst->project_away_var[i] ? 'P' : 'K'));
    }
    printf(" }\n}\n");
}

/* compute abstraction of state and store in abst_state */
static void abstract_state( const abstraction_t *abst, const state_t *state,
                            state_t* abst_state)
{
    int i;
    for( i = 0; i < NUMVARS; ++i ) {
        if( abst->project_away_var[ i ] ) {
            abst_state->vars[ i ] = 0;
        } else {
            abst_state->vars[ i ]
                = abst->value_map[ var_domains[ i ] ][ state->vars[ i ] ];
        }
    }
}

/*--------------------------------------------------------------------------*/

#ifdef HAVE_FWD_PRUNING

void print_fwd_internal_seq(int i)
{
    int j;
    int printed = 0;
    printf("(");
    for (j = 0; 
         (j < fwd_history_len) && (fwd_internal_sequences[i][j] != -1); 
         ++j) 
    {
        if (printed) 
            printf(",");
        printf("%s", fwd_rule_names[fwd_internal_sequences[i][j]]);
        printed = 1;
    }
    printf(")");
}

void print_fwd_prune_seq(int i)
{
    int j;
    int printed = 0;
    printf("(");
    for (j = 0; 
         (j < fwd_history_len+1) && (fwd_prune_sequences[i][j] != -1); 
         ++j) 
    {
        if (printed) 
            printf(",");
        printf("%s", fwd_rule_names[fwd_prune_sequences[i][j]]);
        printed = 1;
    }
    printf(")");
}

#endif

#ifdef HAVE_BWD_PRUNING

void print_bwd_internal_seq(int i)
{
    int j;
    int printed = 0;
    printf("(");
    for (j = 0; 
         (j < bwd_history_len) && (bwd_internal_sequences[i][j] != -1); 
         ++j) 
    {
        if (printed) 
            printf(",");
        printf("%s", bwd_rule_names[bwd_internal_sequences[i][j]]);
        printed = 1;
    }
    printf(")");
}

void print_bwd_prune_seq(int i)
{
    int j;
    int printed = 0;
    printf("(");
    for (j = 0; 
         (j < bwd_history_len+1) && (bwd_prune_sequences[i][j] != -1); 
         ++j) 
    {
        if (printed) 
            printf(",");
        printf("%s", bwd_rule_names[bwd_prune_sequences[i][j]]);
        printed = 1;
    }
    printf(")");
}
#endif

