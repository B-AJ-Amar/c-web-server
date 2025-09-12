<!DOCTYPE html>
<html>
<head>
    <title>Test PHP Request Methods</title>
</head>
<body>
    <h1>Test PHP Request Methods</h1>
    <?php if ($_SERVER['REQUEST_METHOD'] === 'POST'): ?>
        <p>You sent a <strong>POST</strong> request.</p>
    <?php elseif ($_SERVER['REQUEST_METHOD'] === 'GET' && !empty($_GET)): ?>
        <p>You sent a <strong>GET</strong> request.</p>
    <?php endif; ?>

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