<?php

require 'util.php';

$conf = read_config(AUTODEFLECT_ROOT . AUTODAEMON_CONF);

$out = array();
if (file_exists($conf['directory_script'] . "/" . $conf['process_file'] . $conf['pid_suffix'])) {
	$timestamp = file_get_contents($conf['directory_script'] . "/" . $conf['process_file'] . $conf['pid_suffix']); 
	$out['running'] = trim($timestamp);

} else {
	$out['running'] = 0;
}

if (file_exists(AUTODEFLECT_ROOT . "/config/rundata/.completed_timestamp")) {
	$timestamp = file_get_contents(AUTODEFLECT_ROOT . "/config/rundata/.completed_timestamp");
	$out['last_success'] = trim($timestamp) * 1000;
} else {
	$out['last_success'] = 0;
}

if (file_exists(AUTODEFLECT_ROOT . "/config/rundata/.aw_lastrun")) {
	$timestamp = file_get_contents(AUTODEFLECT_ROOT . "/config/rundata/.aw_lastrun");
	$out['last_awstats'] = trim($timestamp) * 1000;
} else {
	$out['last_awstats'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2p" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_process'] = 1;
} else {
	$out['autodeflect_process'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2ssh" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_sshkey'] = 1;
} else {
	$out['autodeflect_sshkey'] = 0;
}

if (file_exists($conf['directory_run'] . "/ad2runner" . $conf['pid_suffix'] . ".pid")) {
	$out['autodeflect_runner'] = 1;
} else {
	$out['autodeflect_runner'] = 0;
}

$maint_mode = glob(AUTODEFLECT_ROOT . "/config/rundata/.maint_lock-*");
if (count($maint_mode) > 0) {
	$out['maintenance'] = 1;
} else {
	$out['maintenance'] = 0;
}

if (isset($_GET["pretty"]))
	echo json_encode($out,JSON_PRETTY_PRINT);
else
	echo json_encode($out);

?>
