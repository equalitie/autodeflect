<?php

$edgelist_glob = $config['edgemanage_root'] . '/edges/*';

$edge_ip_array = array();
foreach (glob("$edgelist_glob") as $dnet) {
        if (is_file($dnet)) {
                $fh = fopen($dnet, 'r');
                $data = trim(fread($fh, filesize($dnet)));
                fclose($fh);

                $data_array = explode("\n", $data);
		$dnet_name = basename($dnet);
                foreach($data_array as $line) {
                        $line = trim($line);
                        if (substr($line,0,1) == '#')
                                continue;
                        $ip = gethostbyname($line);
                        if (filter_var($ip, FILTER_VALIDATE_IP))
                                array_push($edge_ip_array["$dnet_name"],$ip);
                }
        }
}

$out = array();

if (isset($_GET["pretty"]))
	print json_encode($out,JSON_PRETTY_PRINT);
else
	print json_encode($out);

?>
