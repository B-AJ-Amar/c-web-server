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

        <h2>Request Body (POST Data):</h2>
        <pre><?php
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            $body = file_get_contents('php://input');
            if (!empty($body)) {
                echo htmlspecialchars($body);
            } else {
                echo "No POST data received";
            }
        } else {
            echo "Not a POST request";
        }
        ?></pre>

        <h2>POST Form Data:</h2>
        <ul>
            <?php if (!empty($_POST)): ?>
                <?php foreach ($_POST as $key => $value): ?>
                    <li><strong><?php echo htmlspecialchars($key); ?>:</strong> <?php echo htmlspecialchars($value); ?></li>
                <?php endforeach; ?>
            <?php else: ?>
                <li>No POST data submitted</li>
            <?php endif; ?>
        </ul>

    <form action="test.php" method="get">
        <input type="hidden" name="a" value="b">
        <input type="hidden" name="c" value="123">
        <input type="hidden" name="q" value="test">
        <button type="submit">Send GET Request</button>
    </form>
    
    <form action="test.php" method="post">
        <h3>POST Form with Default Data:</h3>
        <label for="name">Name:</label>
        <input type="text" id="name" name="name" value="John Doe"><br><br>
        
        <label for="email">Email:</label>
        <input type="email" id="email" name="email" value="john@example.com"><br><br>
        
        <label for="message">Message:</label>
        <textarea id="message" name="message" rows="4" cols="50">Hello from the test form!</textarea><br><br>
        
        <label for="category">Category:</label>
        <select id="category" name="category">
            <option value="general" selected>General</option>
            <option value="support">Support</option>
            <option value="feedback">Feedback</option>
        </select><br><br>
        
        <input type="hidden" name="form_id" value="test_form_123">
        <button type="submit">Send POST Request with Data</button>
    </form>
</body>
</html>