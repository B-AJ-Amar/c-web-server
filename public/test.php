<!DOCTYPE html>
<html>
<head>
    <title>Test PHP Request Methods</title>
</head>
<body>
    <h1>Test PHP Request Methods</h1>
    Request method is : <?php echo $_SERVER['REQUEST_METHOD']; ?>

    <?php if (!empty($_GET)): ?>
        <h2>Query Parameters (GET):</h2>
        <ul>
            <?php foreach ($_GET as $key => $value): ?>
                <li><strong><?php echo htmlspecialchars($key); ?>:</strong> <?php echo htmlspecialchars($value); ?></li>
            <?php endforeach; ?>
        </ul>
    <?php endif; ?>

    <form action="test.php" method="get">
        <button type="submit">Send GET Request</button>
    </form>
    <form action="test.php" method="post">
        <button type="submit">Send POST Request</button>
    </form>
</body>
</html>