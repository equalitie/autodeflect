<?php

$config = require( 'config/config.php' );
require( 'vendor/autoload.php' );

// Lets get $arg first
$arg = arguments($argv);

$version = trim(file_get_contents('VERSION', true));

// Show help and exit. Warning --help -h, both trigger -h
if (isset($arg['help']) || isset($arg['h'])) {
  show_help($version);
  exit(0);
}

if (isset($arg['show-conf']) || isset($arg['s'])) {
  show_conf($config);
  exit(0);
}

// Set our verbose, default 0, ie; --verbose=1 or -v
if (isset($arg['verbose']) || isset($arg['v'])) {
  if (@$arg['verbose'] > 1)
    define('VERBOSE', 2);
  else
    define('VERBOSE', 1);
} else {
  define('VERBOSE', 0);
}

// -i insert. Logstash or output dir. Default to 1, do not insert.
if (isset($arg['i']))
  define('INSERT', 0);
else
  define('INSERT', 1);

// Special --site= to just run for a single site
if (isset($arg['site']))
  $mysite = $arg['site'];

// Define some paths to commands
define('GREP', '/bin/grep');
define('AWK', '/usr/bin/awk');
define('XARGS', '/usr/bin/xargs');
define('HOST', '/usr/bin/host');
define('CURL', '/usr/bin/curl');
define('SSH', '/usr/bin/ssh');
define('SSH_ADD', '/usr/bin/ssh-add');

// Read in a json list of clients
$sites_array = [];
$sites_array = get_sites_array($config['siteslist']);

// Read cliets list and get dnet information
$dnets_array = [];
$dnets_array = get_dnets_array($sites_array);

$site_checked_array = [];

// Build the array with all sites and assigned dnet data
$cnt = 0;
$data_for_processes = [];
foreach($sites_array->sites as $sites) {
  if(isset($mysite)) {
    if ("$sites->site" == "$mysite") {  
      $data_for_processes[$cnt][$sites->site] = array($sites, $dnets_array[$sites->network]);
      $found = 1;
      break;
    }
  } else {
    $data_for_processes[$cnt][$sites->site] = array($sites, $dnets_array[$sites->network]);
    $cnt++;
  }
}
if (!(isset($found) && $found == 1 || $cnt > 0)) {
  print "No sites found. Exiting\n";
  exit(0);
}
 
if (VERBOSE > 0)
  printf ("Total sites found for processing: %s\n", ($cnt + 1)); 

if (isset($arg['mode']) && $arg['mode'] == 2 && INSERT == 0) {
  if (file_exists($config['image_dir']) && !is_dir($config['image_dir'])) {
    printf("ERROR: Image directory %s exists but is not a directory\n", $config['image_dir']);
    exit(1);
  } elseif (!file_exists($config['image_dir']) && !is_dir($config['image_dir'])) {
    // create image directory, recursive
    if(!(@mkdir($config['image_dir'], 0755, true))) {
      printf("ERROR: Cound not create image directory %s\n", $config['image_dir']);
      exit(1);
    }
  }
}

// Frequency to check if process is done in loop
$check_frequency = 1; // Seconds

// Array of pids
$launched_pids = [];

while(count($data_for_processes) > 0) {
  $data_for_next_process = array_shift($data_for_processes);

  // fork
  $pid = pcntl_fork();

  if ($pid == -1) {
    // Well, if we can't fork then we must die. Not much else we can do.
    die("Unable to fork\n");
  } else if ($pid) {
    // Fork was success. This is parent.
    $launched_pids[] = $pid;

    while (count($launched_pids) >= $config['max_async_check']) {
      sleep($check_frequency);
      $launched_pids = handle_completed_subprocesses($launched_pids);
    }
  } else {
    // This is child.
    $site = array_keys($data_for_next_process)[0];
    if (VERBOSE > 0)
      printf("Start site: %s\n", $site);

    if (isset($arg['mode']))
      $site_checked_array = site_check_array($data_for_next_process[$site][0], $data_for_next_process[$site][1], $arg['mode']);
    else
      $site_checked_array = site_check_array($data_for_next_process[$site][0], $data_for_next_process[$site][1]);

    if (VERBOSE > 1)
      $site_checked_json = json_encode($site_checked_array, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT);
    else
      $site_checked_json = json_encode($site_checked_array, JSON_UNESCAPED_UNICODE);

    if(!submit_ls_query($site_checked_json))
      printf ("ERROR: Failed to insert %s to logstash.\n", $site);

    if (VERBOSE > 0)
      printf("Finish site: %s\n", $site);

    exit(0);
  }
}

while (count($launched_pids) > 0) {
  sleep($check_frequency);
  $launched_pids = handle_completed_subprocesses($launched_pids);
}

exit(0);
///////////////////////////////////////////////////////////////////////////////////
// functions

function check_ssh_agent($mode = 0)
{
  global $config;

  if (file_exists($config['ssh_auth_sock']) && filetype($config['ssh_auth_sock']) == 'socket' && is_readable($config['ssh_auth_sock'])) {

    $cmd = "export SSH_AUTH_SOCK=\"".$config['ssh_auth_sock']."\" ; ".SSH_ADD." -l | " .AWK. " '{print $2}' | " .GREP. " ".$config['ssh_key_fingerprint'];

    exec($cmd, $q, $retval);

    if ($retval == 0) {
      return true;
    }
  }
  return false;
}

function site_check_array($site_obj, $dnet_obj, $mode = 0)
{
  global $config;

  if ((boolVal($site_obj->https)) == true) { 
    $scheme = 'https';
    $port = '443';
  } else { 
    $scheme = 'http';
    $port = '80';
  }

  if ($mode == 2) {
    if (output_image($site_obj->site, $scheme))
      return true;
    else
      return false;
  }

  $out_array = [];
  $cnt = 0;
  // Loop site over dnet edges
  foreach ($dnet_obj['edges'] as $edge) {
    if ($mode == 0) {
     $cmd = CURL." -k -I -s -L --max-redirs 5 --url $scheme://$site_obj->site/ --resolve $site_obj->site:$port:$edge[1] -A \"".$config['user_agent']."\" -m ".$config['max_t']." -w \"%{http_code}\\n%{time_total}\\n%{num_redirects}\\n%{url_effective}\\n\" -o /dev/null";

  } elseif ($mode == 1) {
    if(check_ssh_agent(1) == false) {
      die("ERROR: SSH auth not available\n");
    }

    $pre_cmd = "export SSH_AUTH_SOCK=\"".$config['ssh_auth_sock']."\" ; ".SSH. " -q -o ConnectTimeout=".$config['ssh_connect_t']." -o BatchMode=yes ".$config['ssh_user']."@".$edge[1]." 2>/dev/null ";
    $post_cmd = addslashes(CURL." -k -I -s -L --max-redirs 5 --url $scheme://$site_obj->site/ --resolve $site_obj->site:$port:$site_obj->origin -A \"".$config['user_agent']."\" -m ".$config['max_t']." -w \"%{http_code}\\n%{time_total}\\n%{num_redirects}\\n%{url_effective}\\n\" -o /dev/null");
    $cmd = $pre_cmd."\"".$post_cmd."\"";
  } else {
    die ("ERROR: Unknown --mode=$mode\n");
  }

    exec("$cmd", $out, $s);

    // https://curl.haxx.se/libcurl/c/libcurl-errors.html
    switch ($s) {
      case 0: // Success
        break;
      case 7: // Failed to connect(). Output okay.
      case 28: // Operation timeout. Output okay.
      case 35: // SSL/TLS handshake problem. Output okay.
      case 56: // Failure with receiving network data. Output okay.
        $out[0] = ((int)$s + 700);
        break;
      case 255: // ssh probably failed if in mode=1
        $out[0] = '700';
        if (VERBOSE > 0)
          printf ("ERROR: %s exit returned 'ssh connect or execution error' on site %s at edge %s\n", $s,$site_obj->site,$edge[0]);
        break;
      case 6:
        $out[0] = ((int)$s + 700);
        if (VERBOSE > 0)
          printf("ERROR: curl returned couldn't resolve site %s at edge %s\n", $site_obj->site,$edge[0]);
        break;
      default:
        printf("ERROR: Unknown exit status %s on site %s at edge %s\n", $s,$site_obj->site,$edge[0]);
        // Print this so we can add to above error catching
        print_r($out);
        $continue = 1;
        break;
    }

    if (isset($continue)) {
      print "Skipping ..\n";
      unset ($continue);
      continue;
    }

    if (!(isset($out[0]))) {
      // Not sure how we got here. Print error and skip.
      printf("ERROR: Should never get here. Exit status %s on site %s at edge %s. Skipping ...\n", $s,$site_obj->site,$edge[0]);
      continue;
    }

    if (!(isset($out[1])))
      $out[1] = '0.00';
    if (!(isset($out[2])))
      $out[2] = '0';
    if (!(isset($out[3])))
      $out[3] = 'N/A';

    $out_array[$cnt]['@timestamp'] = (string)date("c");
    $out_array[$cnt]['mode'] = (int)$mode;
    $out_array[$cnt]['client_request_host'] = $site_obj->site;
    $out_array[$cnt]['dnet'] = $site_obj->network;
    $out_array[$cnt]['edge_name'] = $edge[0];
//    $out_array[$cnt]['host_ip'] = $edge[1];
    $out_array[$cnt]['http_response_code'] = $out[0]; 
    $out_array[$cnt]['time_total'] = (float)$out[1];
    $out_array[$cnt]['num_redirects'] = (int)$out[2];
//    $out_array[$cnt]['url_effective'] = $out[3];

    $cnt++;
    unset($out);
  }

  return $out_array;
}

function get_dnets($arrayOfObjects)
{
  if (empty($arrayOfObjects))
    return [];

  $map = [];
  foreach ($arrayOfObjects->sites as $object) {
    $map[$object->network] = true;
  }

  return array_keys($map);
}

function get_edges_array($dnet)
{
  global $config;
// populate a array of edge name,ips in $edges_array
  $cmd = GREP." -v '^#' ".$config['edgefiles']."/".$dnet." | ".AWK." '{print $1}' | ".XARGS." -n 1 ".HOST." | ".AWK." '{print \$1\",\"\$NF}'";

  exec("$cmd", $edges_array, $status);

  if ($status == 0) {
    $ret_array = array();
    foreach($edges_array as $value)
      array_push($ret_array, (explode(',',$value)));
  } else {
    $ret_array = false;
  }

  return $ret_array;
}

function get_sites_array($siteslist)
{
  $sites_content = @file_get_contents($siteslist);
  if ($sites_content === FALSE) {
    printf("ERROR: Problem reading file %s", $siteslist);
    exit(1);
  }

  $sites_content = utf8_encode($sites_content);
  $sites_array = [];
  $sites_array = @json_decode($sites_content);
  if ($sites_array === NULL) {
    printf("ERROR: Syntax problem in json file %s", $siteslist);
    exit(1);
  }

  return $sites_array;
}

function get_dnets_array($sites_array)
{
  $dnetsList = [];
  $dnetsList = get_dnets($sites_array);

  foreach ($dnetsList as $dnet) {
    $dnets_array[$dnet]['edges'] = get_edges_array($dnet);
  }

  return $dnets_array;
}

function handle_completed_subprocesses($pids) {
  foreach($pids as $index => $pid) {
    $res = pcntl_waitpid($pid, $status, WNOHANG);
    // Three possible return values:
    // -1: That's not good
    //  0: The specified process hasn't exited yet
    //  (same as the supplied process id): The process has exited
    if ($res == -1) {
      // FIXME: Do something because there was a error and the proccess died.
      unset($pids[$index]);
    } else if ($res == $pid) {
        // Looks like process $pid has finished - removing from list
        unset($pids[$index]);
      }
    }
  return $pids;
}

// Sumit document to logstash
function submit_ls_query($json)
{
  global $config;

  // create var $indexUrl .. create 1 - index, 2 - mapping, 3 - insert json
  $indexUrl = 'https://'.$config['ls_user'].':'.$config['ls_password'].'@'.$config['ls_host'].':'.$config['ls_port'].'/'.$config['ls_path'];

  if (VERBOSE > 1)
    printf("%s\n", $json);

  if (INSERT > 0)
    return true;

  $ci = curl_init();
  curl_setopt($ci, CURLOPT_URL, $indexUrl);
  curl_setopt($ci, CURLOPT_PORT, $config['ls_port']);
  curl_setopt($ci, CURLOPT_TIMEOUT, $config['ls_timeout']);
  curl_setopt($ci, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($ci, CURLOPT_FORBID_REUSE, 0);
  curl_setopt($ci, CURLOPT_CUSTOMREQUEST, 'POST');
  curl_setopt($ci, CURLOPT_POSTFIELDS, "$json");
  $response = curl_exec($ci);
  curl_close($ci);

  switch (strtolower($response)) {
    case 'ok':
      $ret = true;
      break;
    default:
      $ret = false;
  }

  return $ret;
}

function boolVal($var)
{
  if (!is_string($var)) return (bool) $var;

  switch (strtolower($var)) {
    case 'true':
    case 'on':
    case 'yes':
    case 'y':
      $out = true;
      break;
    default:
      $out = false;
  }

  return $out;
}

function arguments($argv)
{
  $_ARG = [];
    foreach ($argv as $arg) {
      if (ereg('--([^=]+)=(.*)',$arg,$reg)) {
        $_ARG[$reg[1]] = $reg[2];
      } elseif(ereg('-([a-zA-Z0-9])',$arg,$reg)) {
            $_ARG[$reg[1]] = 'true';
        }
  
    }
  return $_ARG;
}

// works but needs work
function output_image($site, $scheme)
{
  //use JonnyW\PhantomJs\Client;
  global $config;

  if (!is_dir($config['image_dir']) || !is_writable($config['image_dir'])) {
    printf("ERROR: Problem with %s\n", $config['image_dir']);
    exit(1);
  }

  $myfile = $config['image_dir']."/".$site."-".time().".".$config['image_type'];

  if (VERBOSE > 0)
    print $myfile."\n";


  $client = \JonnyW\PhantomJs\Client::getInstance();
  $client->getEngine()->setPath($config['phantomjs']);

  $top    = 0;
  $left   = 0;
  $width  = $config['image_width'];
  $height = $config['image_height'];;

  $request = $client->getMessageFactory()->createCaptureRequest("$scheme://$site", 'GET');
  $request->setOutputFile($myfile);
  $request->setViewportSize($width, $height);
  $request->setCaptureDimensions($width, $height, $top, $left);

  /** 
   * @see JonnyW\PhantomJs\Http\Response 
   **/
  $response = $client->getMessageFactory()->createResponse();

  if (VERBOSE > 1) {
    print "request:\n";
    print_r($request);
    print "\nresponce:\n";
    print_r($response);
    print "\nclient:\n";
    print_r($client);
    print "\n";
  }


  if (INSERT > 0)
    return true;
  // Send the request
  $client->send($request, $response);

  return true;

}

function show_conf($config)
{

// FIXME: make this better in future

  // Change ls_password. Still not secure, but better than just printing it here
  $config['ls_password'] = 'xxxxxxxxxxxxxxxxxxxxxxxx';
  print_r($config);
  print "\n";

}

// help
function show_help($version)
{
  $out = "\nVersion: " .$version."\n\n";

  $out .= <<<EOT
  --help|-h		Print this help
  --verbose=1|-v	Print verbose of level 1
  --verbose=2		Print verbose of level 2
  --site=<site.name>	--site=example.co will run a single site
  --insert|-i		Insert to logstash or
			output to directory if --mode=2
			Default to 'No insert'
  --mode=0|1|2		Default mode=0 (run local)
				mode=1 (run remote, on edges via ssh)
				mode=2 (run image output mode)		
  --show-conf|-s	Print out config and exit

  Note: On many errors due to connections this program will modify the
        json output. In most case just changing the "http_response_code":
        to curl's exit code + 700. If in --mode=1 and ssh can not connect
        to remote host (exit code 255) the output will change to
        "http_response_code": 700 .

EOT;

  printf("%s\n", $out);

  return true;
}

?>
