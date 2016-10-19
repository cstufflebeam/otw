#!/usr/bin/perl
use strict;

my @validchars=split(//,"12345678");
my @locks;
my $tries=0;

sub addToLocks {
    my $i=0;
    foreach my $tmp (@_) {
	$locks[$i++]+=$tmp;
    }
}
sub resLocks {
    for(my $i=0;$i<5;$i++) {
	$locks[$i]=300;
    }
}

sub printLocks {
    my ($attempt)=@_;
    print "$attempt\t";
    for(my $i=0;$i<5;$i++) {
	printf("%d\t",$locks[$i]);
    }
    printf("\n");
}
sub manipulateLocks {
    my ($num)=@_;
    for ($num) {
	/1/ && do {addToLocks(5,2,1,7,5);last;};
	/2/ && do {addToLocks(13,-7,-4,1,5);last;};
	/3/ && do {addToLocks(9,12,9,70,-4);last;};
	/4/ && do {addToLocks(-11,9,0,5,-13);last;};
	/5/ && do {addToLocks(4,17,12,9,24);last;};
	/6/ && do {addToLocks(11,-17,21,5,14);last;};
	/7/ && do {addToLocks(15,31,22,-12,3);last;};
	/8/ && do {addToLocks(19,-12,4,3,-7);last;};
	/r/ && do {resLocks();last;};
    }
}

sub checkLocks {
    for(my $i=0;$i<5;$i++) {
	if($locks[$i]!=400) {
	    return 0;
	}
    }
    return 1;
}

sub bf {
    my ($target,$attempt,$position,$skip)=@_;
    my @newattempt=split(//,$attempt);
    if($position<0) {
	return;
    }
    for(my $i=$skip;$i<@validchars;$i++) {
 	resLocks();
	$tries++;
	$newattempt[$position]=$validchars[$i];
	for(my $j=0;$j<int(@newattempt);$j++) {
	    manipulateLocks($newattempt[$j]);
	}
	my $print=0;
	for(my $k=0;$k<5;$k++) {
	    if($locks[$k]==400) {
		$print++;
	    }
	}
	if($print>=2) {
	    print "Tried $tries so far - ";
	    printLocks(join(/ /,@newattempt));
	}
	if(checkLocks()) {
	    print "Tried $tries so far - ";
	    printLocks(join(/ /,@newattempt));
	    print("Found with ".join(/ /,@newattempt).".\n");
	    exit(0);
	}
	bf($target,join(/ /,@newattempt),$position-1,$i);
    }
}

print "Brute forcing with max of $ARGV[0] numbers.\n";
my $numchars=$ARGV[0];
my @first;
for(my $i=0;$i<$numchars;$i++) {
    $first[$i]=0;
}
resLocks();
bf($ARGV[0],join(/ /,@first),$numchars-1,0);
