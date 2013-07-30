
drop function demule(text);

CREATE FUNCTION demule(text) RETURNS text AS '
    my $original=shift;
    $original =~  s|[]||g;
    return $original;
' LANGUAGE 'plperl';




CREATE FUNCTION demule_202(text) RETURNS text AS '
    my $original=shift;
    $original =~  s|[]||g;
    return $original;
' LANGUAGE 'plperl';



CREATE FUNCTION demule_214(text) RETURNS text AS '
    my $original=shift;
    $original =~  s|[]||g;
    return $original;
' LANGUAGE 'plperl';




drop function mule(int,text);
CREATE FUNCTION mule(int,text) RETURNS text AS '
my ($tag,$original)=@_;
my  $ttag=chr($tag);
$original =~ s/([\\\x7f-\\\xff])/$ttag$1/g;
# s/([\x7f-\xff])/$ttag$1/g;
return $original;
' LANGUAGE 'plperl';


