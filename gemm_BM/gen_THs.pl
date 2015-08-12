#!/usr/bin/perl

$perf_path = "./logs";

$m=16;
$mkl=1;

$max_perf = 0;
$max_n = 0;
@MSTH_array=qw();
@MLTH_array=qw();
$count = 0;

for ( $k=2;$k<=16384;$k*=2)
{
    $max_perf = 0;
    $max_n = 0;

# Find the maximum perf on different ns
    for ( $n=2;$n<=16384;$n*=2)
    {
      $tmp_perf = 0;
      $perf_file="${perf_path}/mkl${mkl}/perf_${m}_${k}_${n}.txt";
      open(PERF_FILE, $perf_file) || die "Cannot open $perf_file : $!\n";
      while ($line = <PERF_FILE>)
      {
        chomp $line;
        @words = split (/\ /, $line);  
        if ($words[-2] eq "perf:" )
        {
          $tmp_perf = $words[-1];
          last;
        }
      }
      close(PERF_FILE);
      if ( $tmp_perf > $max_perf )
      {
        $max_perf = $tmp_perf;
        $max_n = $n;
      }
    }

# Determine MSTH
    $MSTH_perf = 0;
    $MSTH = 0;
    for ( $n=2;$n<=16384;$n*=2)
    {
      $tmp_perf = 0;
      $perf_file="${perf_path}/mkl${mkl}/perf_${m}_${k}_${n}.txt";
      open(PERF_FILE, $perf_file) || die "Cannot open $perf_file : $!\n";
      while ($line = <PERF_FILE>)
      {
        chomp $line;
        @words = split (/\ /, $line);  
        if ($words[-2] eq "perf:" )
        {
          $tmp_perf = $words[-1];
          last;
        }
      }
      close(PERF_FILE);
      if ( $tmp_perf < 0.8*$max_perf )
      {
        if ( $n < $max_n )
        {
          $MSTH_perf = $tmp_perf;
          $MSTH_n = $n;
          $MSTH = ($n * $k + $m * $n + $m * $k)* 8/1.0e6;
        }
      }
    }
# Determine MLTH
    $MLTH_perf = 0;
    $MLTH = 0;
    for ( $n=16384;$n>=2;$n/=2)
    {
      $tmp_perf = 0;
      $perf_file="${perf_path}/mkl${mkl}/perf_${m}_${k}_${n}.txt";
      open(PERF_FILE, $perf_file) || die "Cannot open $perf_file : $!\n";
      while ($line = <PERF_FILE>)
      {
        chomp $line;
        @words = split (/\ /, $line);  
        if ($words[-2] eq "perf:" )
        {
          $tmp_perf = $words[-1];
          last;
        }
      }
      close(PERF_FILE);
      if ( $tmp_perf < 0.8*$max_perf )
      {
        if ( $n > $max_n )
        {
          $MLTH_perf = $tmp_perf;
          $MLTH_n = $n;
          $MLTH = ($n * $k + $m * $n + $m * $k)* 8/1.0e6;
        }
      }
    }
#    printf ("m${m}_k${k}:\n");
#    printf ("max_perf: %.2f, max_n: %d\n", $max_perf, $max_n);
#    printf ("MSTH_perf: %.2f, MSTH_n: %d, MSTH: %.2f MB\n", $MSTH_perf, $MSTH_n, $MSTH);
#    printf ("MLTH_perf: %.2f, MLTH_n: %d, MLTH: %.2f MB\n", $MLTH_perf, $MLTH_n, $MLTH);
    $MSTH_array[$count] = $MSTH;
    $MLTH_array[$count] = $MLTH;
    $count ++;
}

$sum_MSTH = 0;
$sum_MLTH = 0;
for ( $j=0;$j<$count;$j++)
{
  $sum_MSTH += $MSTH_array[$j];
  $sum_MLTH += $MLTH_array[$j];
}
$aver_MSTH = $sum_MSTH / $count;
$aver_MLTH = $sum_MLTH / $count;
printf ("count: %d\n", $count);
printf ("aver_MSTH: %.2f MB\n", $aver_MSTH);
printf ("aver_MLTH: %.2f MB\n", $aver_MLTH);

$out_file=">>../defines.h";
open(OUT_FILE, $out_file) || die "Cannot open $out_file : $!\n";
printf OUT_FILE ("#define MSTH %.2f\n", $aver_MSTH);
printf OUT_FILE ("#define MLTH %.2f\n", $aver_MLTH);
close(OUT_FILE);
