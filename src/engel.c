/****************************************************************************
**
**    engel.c                         NQ                       Werner Nickel
**                                         Werner.Nickel@math.rwth-aachen.de
*/


#include "nq.h"
#include "engel.h"

static  gen     **CommuteList = (gen**)0;

static	int	LeftEngel = 0, RightEngel = 0, Engel = 0;
static  int     RevEngel = 0;
static  int     NrEngelGens = 0;
static	int	NrWords;
static  int     Needed;
static	word	A;
        int     SemigroupOnly  = 0;
        int     SemigroupFirst = 0;
        int     CheckFewInstances = 0;
        int     ReverseOrder = 0;

static
void	Error( v, w, type )
word	v, w;
char	type;

{	printf( "Overflow in collector computing [ " );
	printWord( v, 'a' );
	if( type == 'e' ) printf( " , %d ", Engel );
	if( type == 'l' ) printf( " , %d ", LeftEngel );
	if( type == 'r' ) printf( " , %d ", RightEngel );
	printWord( w, 'a' );
	printf( " ]\n" );
}

static
void	evalEngelRel( v, w )
word	v, w;

{	word	vs = v, ws = w, v1, vv = v;
	long	n = 0, needed;

        gen    *SaveCommute = Commute;

/*	printf( "evalEngelRel() called with : " );
	printWord( v, 'A' ); printf( "    " );
	printWord( w, 'A' ); putchar( '\n' ); */

	NrWords++;
	/* Calculate [ v, w, .., w ] */
        if( Class+1 > Engel ) Commute = CommuteList[n];
	if( (v = Commutator( v, w )) == (word)0 ) {
	    Error( vv, w, 'e' );
	    return;
	}
	n++;
	while( n < Engel ) {
            if( Class+1 > Engel ) Commute = CommuteList[n];
          
	    if( (v1 = Commutator( v, w )) == (word)0 ) {
		Error( vv, w, 'e' );
		free( v );
		return;
	    }
	    free( v );
	    v = v1;

            n++;
	}
        Commute = SaveCommute;

	needed = addRow( ExpVecWord( v ) );
	if( needed ) {
   	    printf( "#    [ " );
	    printWord( vs, 'a' );
	    for( n = Engel-1; n >= 0; n-- ) {
		printf( ", " );
	    	printWord( ws, 'a' );
	    }
	    printf( " ]\n" );
	}
        if( CheckFewInstances ) Needed |= needed;
        else                    Needed = 1;

	free( v );
}

static
void	buildPairs( u, i, g, v, wt, which )
word	u, v;
long	i;
gen	g;
long	wt, which;

{	long	save_wt;


        /* First we check if the Engel condition is trivially
           satisfied for weight reasons. The commutator
           [u, n v] is 1 if w(u) + n*w(v) > Class+1. */
        if( which == 1 && i == 1 &&
           Wt(abs(u[0].g)) + Engel*Wt(abs(v[0].g)) > Class+1 )
            return;

	if( wt == 0 && which == 1 && i > 0 ) {
	    evalEngelRel( u, v );
	    return;
	}

	/* Keep u and start to build v. */
	if( i > 0 && which == 2 ) buildPairs( v, 0, 1, u, wt, 1 );

	if( g > NrPcGens ) return;

	save_wt = wt;
	while( !EarlyStop && 
               g <= NrPcGens && Wt(g) <= Class+1 - Engel && Wt(g) <= wt ) {
	    u[i].g   = g;
	    u[i].e   = (exp)0;
 	    u[i+1].g = EOW;
	    while( !EarlyStop && Wt(g) <= wt ) {
		u[i].e++;
		if( Exponent[g] > (exp)0 && Exponent[g] == u[i].e ) break;
		wt -= Wt(g);
		buildPairs( u, i+1, g+1, v, wt, which );
                /* now build the same word with negative exponent */
                if( !EarlyStop && !SemigroupOnly && Exponent[g] == (exp)0 ) {
                    u[i].g *= -1;
                    buildPairs( u, i+1, g+1, v, wt, which );
                    u[i].g *= -1;
                }
	    }
	    wt = save_wt;
	    g++;
	}
	u[i].g = EOW;
	u[i].e = (exp)0;
	if( EarlyStop || SemigroupOnly || !SemigroupFirst ) return;

	while( !EarlyStop && 
               g <= NrPcGens && Wt(g) <= Class+1 - Engel && Wt(g) <= wt ) {
	    u[i].g   = -g;
	    u[i].e   = (exp)0;
 	    u[i+1].g = EOW;
	    while( !EarlyStop && Wt(g) <= wt ) {
		u[i].e++;
		if( Exponent[g] > (exp)0 && Exponent[g] == u[i].e ) break;
		wt -= Wt(g);
		buildPairs( u, i+1, g+1, v, wt, which );
                if( EarlyStop ) return;
                /* now build the same word with negative exponent */
                if( !EarlyStop && !SemigroupOnly && Exponent[g] == (exp)0 ) {
                    u[i].g *= -1;
                    buildPairs( u, i+1, g+1, v, wt, which );
                    u[i].g *= -1;
                }
	    }
	    wt = save_wt;
	    g++;
	}
	u[i].g = EOW;
	u[i].e = (exp)0;
}

static
void	evalEngel()

{	word	u, v;
	long	c;

	u = (word)Allocate( (NrPcGens+NrCenGens+1) * sizeof(gpower) );
	v = (word)Allocate( (NrPcGens+NrCenGens+1) * sizeof(gpower) );

        /* For `production purposes' I don't want to run through       */
        /* those classes that don't yield non-trivial instances of the */
        /* Engel law. Therefore, we stop as soon as we ran through a   */
        /* class that didn't yield any non-trivial instances. This is  */
        /* done through the static variable Needed which is set by     */
        /* evalEngelRel() as soon as a non-trivial instance has been   */
        /* found if the flag CheckFewInstances (option -c) is set.     */
        if( ReverseOrder )   
            for( c = Class+1; !EarlyStop && c >= 2; c-- ) {
                u[0].g = EOW; u[0].e = (exp)0;
                v[0].g = EOW; v[0].e = (exp)0;
                NrWords = 0;
                if(Verbose)
                    printf("#    Checking pairs of words of weight %d\n",c);
                buildPairs( u, 0, 1, v, c, 2 );
                if(Verbose) printf( "#    Checked %d words.\n", NrWords );
            }
        else {
            Needed = 1;
            for( c = 2; !EarlyStop && Needed && c <= Class+1; c++ ) {
                Needed = 0;
                u[0].g = EOW; u[0].e = (exp)0;
                v[0].g = EOW; v[0].e = (exp)0;
                NrWords = 0;
                if(Verbose)
                    printf("#    Checking pairs of words of weight %d\n",c);
                buildPairs( u, 0, 1, v, c, 2 );
                if(Verbose) printf( "#    Checked %d words.\n", NrWords );
            }
            for( ; !EarlyStop && c <= Class+1; c++ )
                printf("#    NOT checking pairs of words of weight %d\n",c);
        }
	free( u ); free( v );
}

static
void	evalRightEngelRel( w )
word	w;

{	word	ws = w, v, v1;
	long	n = 0, needed;
        
        gen     *SaveCommute = Commute;

/*	printf( "evalRightEngelRel() called with : " );*/
/*	printWord( w, 'A' );*/
/*	putchar( '\n' );*/

	NrWords++;
	/* Calculate [ a, w, .., w ] */
        if( Class+1 > RightEngel ) Commute = CommuteList[n];
	if( (v = Commutator( A, w )) == (word)0 ) {
	    Error( A, w, 'r' );
	    return;
	}
	n++;
	while( n < RightEngel ) {
            if( Class+1 > RightEngel ) Commute = CommuteList[n];
          
	    if( (v1 = Commutator( v, w )) == (word)0 ) {
		Error( A, w, 'r' );
		free( v );
		return;
	    }
	    free( v );
	    v = v1;

            n++;
	}
        Commute = SaveCommute;

	needed = addRow( ExpVecWord( v ) );
	if( needed ) {
   	    printf( "#    [ " );
	    printWord( A, 'a' );
	    for( n = RightEngel-1; n >= 0; n-- ) {
		printf( ", " );
	    	printWord( ws, 'a' );
	    }
	    printf( " ]\n" );
	}
        if( CheckFewInstances ) Needed |= needed;
        else                    Needed = 1;

	free( v );
}

static
void	evalLeftEngelRel( w )
word	w;

{	word	ws = w, v, v1;
	long	n = 0, needed;

        gen     *SaveCommute = Commute;

/*	printf( "evalLeftEngelRel() called with : " );*/
/*	printWord( w, 'A' );*/
/*	putchar( '\n' );*/

	NrWords++;
	/* Calculate [ w, a, .., a ] */
        if( Class+1 > LeftEngel ) Commute = CommuteList[n];
	if( (v = Commutator( w, A )) == (word)0 ) {
	    Error( w, A, 'l' );
	    return;
	}
	n++;
	while( n < LeftEngel ) {
            if( Class+1 > LeftEngel ) Commute = CommuteList[n];

	    if( (v1 = Commutator( v, A )) == (word)0 ) {
		Error( w, A, 'l' );
		free( v );
		return;
	    }
	    free( v );
	    v = v1;

            n++;
	}
        Commute = SaveCommute;

	needed = addRow( ExpVecWord( v ) );
	if( needed ) {
   	    printf( "#    [ " );
	    printWord( ws, 'a' );
	    for( n = LeftEngel-1; n >= 0; n-- ) {
		printf( ", " );
	    	printWord( A, 'a' );
	    }
	    printf( " ]\n" );
	}
        if( CheckFewInstances ) Needed |= needed;
        else                    Needed = 1;

	free( v );
}

static
void	buildWord( u, i, g, wt )
word	u;
long	i, wt;
gen	g;

{	long	save_wt;

	if( wt == 0 && i > 0 ) { 
	    if( RightEngel ) evalRightEngelRel( u );
	    if( LeftEngel )  evalLeftEngelRel( u );
	    return;
	}

	if( g > NrPcGens ) return;

	save_wt = wt;
	while( !EarlyStop && g <= NrPcGens && Wt(g) <= wt ) {
	    u[i].g   = g;
	    u[i].e   = (exp)0;
	    u[i+1].g = EOW;
	    while( !EarlyStop && Wt(g) <= wt ) {
		u[i].e++;
		if( Exponent[g] > (exp)0 && Exponent[g] == u[i].e ) break;
		wt -= Wt(g);
		buildWord( u, i+1, g+1, wt );
                /* now build the same word with negative exponent */
                if( !EarlyStop && !SemigroupOnly &&
                    !SemigroupFirst && Exponent[g] == (exp)0 ) {
                    u[i].g *= -1;
                    buildWord( u, i+1, g+1, wt );
                    u[i].g *= -1;
                }
	    }
	    wt = save_wt;
	    g++;
	}
	u[i].g = EOW;
	u[i].e = (exp)0;
        if( EarlyStop || SemigroupOnly || !SemigroupFirst ) return;
	while( !EarlyStop && g <= NrPcGens && Wt(g) <= wt ) {
	    u[i].g   = -g;
	    u[i].e   = (exp)0;
	    u[i+1].g = EOW;
	    while( !EarlyStop && Wt(g) <= wt ) {
		u[i].e++;
		if( Exponent[g] > (exp)0 && Exponent[g] == u[i].e ) break;
		wt -= Wt(g);
		buildWord( u, i+1, g+1, wt );
	    }
	    wt = save_wt;
	    g++;
	}
	u[i].g = EOW;
	u[i].e = (exp)0;
}

static
void	evalLREngel() {

	word	u;
	int	n;
	long	cl;
        extern  unsigned NrGens;

	A = (word)Allocate( 2*sizeof(gpower) );
        u = (word)Allocate( (NrPcGens+NrCenGens+1) * sizeof(gpower) );

        for( n = 1; !EarlyStop && n <= NrEngelGens; n++ ) {

            if( RevEngel ) A[0].g = NrGens - n + 1;
            else           A[0].g = n;
            A[0].e = (exp)1;
	    A[1].g = EOW; A[1].e = (exp)0;

            Needed = 1;
	    for( cl = 2; !EarlyStop && Needed && cl <= Class+1; cl++ ) {
                Needed = 0;
	        u[0].g = EOW; u[0].e = (exp)0;
	        NrWords = 0;
	        if(Verbose) printf("#    Checking words of weight %d\n",cl-1);
	        buildWord( u, 0, 1, cl-1 );
	        if(Verbose) printf( "#    Checked %d words.\n", NrWords );
	    }
            for( ; !EarlyStop && cl <= Class+1; cl++ )
                printf("#    NOT checking words of weight %d\n", cl );

        }
	free( u ); free( A );
}

void    RedefineCommute( weight )
int     weight;

{

    int c;  gen g, h;
    
    if( Class+1 > weight ) {
          
        /* For examining Engel conditions we try a trick with the
           collector. */

        if( CommuteList != (gen**)0 ) Free( CommuteList );
            
        CommuteList = (gen**)Allocate( weight * sizeof(gen*) );
        for( c = 0; c < weight; c++ ) {
            CommuteList[ c ] = 
                (gen*)Allocate( (NrPcGens + NrCenGens + 1) * sizeof(gen) );
              
            /*   We create a Commute list as if the group has class 
            **   `Class+1' - (weight - c - 1)
            */
              
            for( g = 1; g <= NrPcGens; g++ ) {
              for( h = g+1; Wt(g)+Wt(h) <= Class+1 - (weight-c-1); h++ ) ;
              CommuteList[c][g] = h-1;
            }
            for( ; g <= NrPcGens+NrCenGens; g++ ) CommuteList[c][g] = g;
        }
    }

}

void	EvalEngel() {	

	long	t;

	if( Verbose ) t = RunTime();

        /*   If a left Engel and a right Engel condition is used, we have to  
        **   make sure that we use the smaller value for redefining the
        **   Commute array because otherwise we calculate commutators with the
        **   wrong precision.
        */
	if( LeftEngel || RightEngel ) {
            if( !LeftEngel  ) {
                RedefineCommute( RightEngel );
                evalLREngel();
            }
            else if( !RightEngel ) {
                RedefineCommute(  LeftEngel );
                evalLREngel();
            }
            else if( LeftEngel <= RightEngel ) {
                RedefineCommute( LeftEngel );
                evalLREngel();
            }
            else {
                RedefineCommute( RightEngel );
                evalLREngel();
            }
        }

	if( Engel ) {
            RedefineCommute( Engel );
            evalEngel();
        }

	if( Verbose )
	    printf("#    Evaluated Engel condition (%d msec).\n",RunTime()-t);
}

void	InitEngel( l, r, v, e, n )
int	l, r, v, e, n;

{	LeftEngel = l;
	RightEngel = r;
        RevEngel = v;
	
        Engel = e;

        NrEngelGens = n;
}
