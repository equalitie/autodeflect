<?php
require '../vendor/autoload.php';
if (is_readable('../config.php')) {
  $config = require( '../config.php' );
} else {
  print "Not configured";
  exit;
}


$app = new \Slim\Slim(array(
	'templates.path' => '../api',
	'debug' => false,
	)
);

// Define routes
$app->get('/:route', function () use ($app) {
	$app->view(new \Slim\Views\Twig());
	$twig = $app->view->getInstance();
	$loader = $twig->getLoader();
	// This is a addition directory
	$loader->addPath('../templates');
	$app->view->parserOptions = array(
		'charset' => 'utf-8',
		'auto_reload' => true,
		'strict_variables' => false,
		'autoescape' => true
	);
	$app->view->parserExtensions = array(new \Slim\Views\TwigExtension());
	$app->render('index.html');
})->conditions(array("route" => "(|api|api/)"));

$app->get('/api/:name', function ($name) use ($app) {
	$app->response()->header("Content-Type", "application/json");
	$app->response()->header("Cache-Control", "no-cache");
	$app->render('index.php', compact ('app',  'name'));
});

// Run app
$app->run();
