<!DOCTYPE html>
<html>
<head>
    <title>Test PHP Request Methods</title>
</head>
<body>
    <h1>Test PHP Request Methods</h1>
    Request method is : <?php echo $_SERVER['REQUEST_METHOD']; ?>

        <h2>Query Parameters (GET):</h2>
        <ul>
            <?php foreach ($_GET as $key => $value): ?>
                <li><strong><?php echo htmlspecialchars($key); ?>:</strong> <?php echo htmlspecialchars($value); ?></li>
            <?php endforeach; ?>
        </ul>


        <h2>Request Headers:</h2>
        <ul>
            <?php
            foreach (getallheaders() as $name => $value) {
                echo '<li><strong>' . htmlspecialchars($name) . ':</strong> ' . htmlspecialchars($value) . '</li>';
            }
            ?>
        </ul>

    <form action="test.php" method="get">
        <button type="submit">Send GET Request</button>
    </form>
    <form action="test.php" method="post">
        <button type="submit">Send POST Request</button>
    </form>
</body>
</html>