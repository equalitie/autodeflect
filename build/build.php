#!/usr/bin/env php
<?php
$pharfile = 'originhealth.phar';
$startfile = 'src/program.php';
$version = 'VERSION';

$basedir = dirname(dirname( __FILE__ ));

function prunePrefix( $files, $prefix ) {
    $newlist = array();
    $prefix_len = strlen( $prefix );
    foreach( $files as $file => $data ) {
        if( substr( $file, 0, $prefix_len ) !== $prefix ) {
            $newlist[$file] = $data;
        }
    }
    return $newlist;
}
function pruneSuffix( $files, $suffix ) {
    $newlist = array();
    $suffix_len = strlen( $suffix );
    foreach( $files as $file => $data ) {
        if( substr( $file, -$suffix_len ) !== $suffix ) {
            $newlist[$file] = $data;
        }
    }
    return $newlist;
}
$phar = new Phar($pharfile);
$fileIter = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator( $basedir, FileSystemIterator::SKIP_DOTS ) );
$files = iterator_to_array( $fileIter );
$files = prunePrefix( $files, $basedir . '/.git' );
$files = prunePrefix( $files, $basedir . '/build' );
$files = prunePrefix( $files, $basedir . '/composer' );
$files = prunePrefix( $files, $basedir . '/bin' );
$files = pruneSuffix( $files, 'swp' );
$files = pruneSuffix( $files, '~' );
foreach( $files as $file ) {
    echo "Will add $file ...\n";
}
$phar->buildFromIterator( new ArrayIterator($files), $basedir );

$stub = <<<"EOT"
#!/usr/bin/env php
<?php
Phar::mapPhar( '$pharfile' );
set_include_path( 'phar://$pharfile' . PATH_SEPARATOR . 'phar://$version' . PATH_SEPARATOR . get_include_path() );
require('$startfile');
__HALT_COMPILER();
EOT;

$phar->setStub( $stub );
system( "chmod +x $pharfile" );
