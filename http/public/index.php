<?php
require '../vendor/autoload.php';
require '../config.php';

$app = new \Slim\Slim(array(
	'templates.path' => '../api',
	'debug' => false,
	)
);

// Define routes
$app->get('/:route', function () use ($app) {
	$app->view(new \Slim\Views\Twig());
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
