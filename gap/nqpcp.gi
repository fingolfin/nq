##############################################################################
##
#A  nqpcp.gi                    Mai  1999                        Werner Nickel
##
##  This file contains functions for the interface to the share package
##  ``polycyc''.
##

#############################################################################
##
#F  NqInitFromTheLeftCollector  . . . . . . . . . initialise an ftl collector
##
InstallGlobalFunction( NqInitFromTheLeftCollector,
function( nqrec )
    local   ftl,  g,  rel;

    ftl := FromTheLeftCollector( nqrec.NrGenerators );

    for g in [1..nqrec.NrGenerators] do
        SetRelativeOrder( ftl, g, nqrec.RelativeOrders[ g ] );
    od;

    for rel in nqrec.Powers do
        SetPower( ftl, rel[1], rel{[2..Length(rel)]}  );
    od;

    for rel in nqrec.Conjugates do
        SetConjugate( ftl, rel[1], rel[2], rel{[3..Length(rel)]}  );
    od;

    SetFeatureObj( ftl, IsConfluent, true );
    UpdatePolycyclicCollector( ftl );

    return ftl;

end );

#############################################################################
##
#F  NqPcpGroupByCollector . . . . . . . . . pcp group from collector, set lcs
##
InstallGlobalFunction( NqPcpGroupByCollector,
function( coll, nqrec )
    local   G,  gens,  ranks,  lcs,  a,  z,  r;

    G := PcpGroupByCollectorNC( coll );
    gens := GeneratorsOfGroup( G );

    ranks := nqrec.Ranks;
    lcs   := [ G ];

    a     := 1; 
    z     := nqrec.NrGenerators;
    for r in ranks do
        a := a + r;
        Add( lcs, Subgroup( G, gens{[a..z]} ) );
    od;

    SetLowerCentralSeriesOfGroup( G, lcs );
    SetIsNilpotentGroup( G, true );

    return G;
end );


#############################################################################
##
#F  NqPcpElementByWord  . . . . . .  pcp element from generator exponent list
##
InstallGlobalFunction( NqPcpElementByWord, PcpElementByWord );
