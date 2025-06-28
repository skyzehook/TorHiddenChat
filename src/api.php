<?php
header('Content-Type: application/json');
session_start();

define('USER_FILE', __DIR__ . '/userlist.json');
define('CHAT_FILE', __DIR__ . '/messages.json');


if (!file_exists(USER_FILE)) file_put_contents(USER_FILE, json_encode([]));
if (!file_exists(CHAT_FILE)) file_put_contents(CHAT_FILE, json_encode([]));

$users = json_decode(file_get_contents(USER_FILE), true);
$chat = json_decode(file_get_contents(CHAT_FILE), true);

$action = $_GET['action'] ?? '';

switch ($action) {
    case 'register':

        $name = trim($_POST['name'] ?? '');
        if ($name === '') {
            echo json_encode(['error' => 'name = null']);
            exit;
        }

        if (!in_array($name, $users)) {
            $users[] = $name;
            file_put_contents(USER_FILE, json_encode($users));
        }

        echo json_encode(['status' => 'registered', 'name' => $name]);
        break;

    case 'send':
        $name = trim($_POST['name'] ?? '');
        $message = trim($_POST['message'] ?? '');

        if ($name === '' || $message === '') {
            echo json_encode(['error' => 'name and message = null']);
            exit;
        }

        if (!in_array($name, $users)) {
            echo json_encode(['error' => 'user = null']);
            exit;
        }

        $chat[] = [
            'name' => htmlspecialchars($name),
            'message' => htmlspecialchars($message),
            'time' => date('Y-m-d H:i:s')
        ];
        file_put_contents(CHAT_FILE, json_encode($chat));

        echo json_encode(['status' => 'message sending']);
        break;

    case 'fetch':
 
        echo json_encode($chat);
        break;

    default:
        echo json_encode(['error' => 'invalid notification']);
        break;
}
