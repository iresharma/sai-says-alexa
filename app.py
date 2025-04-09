from flask import Flask, request, jsonify, render_template
from flask_cors import CORS
import json
import os

app = Flask(__name__)
CORS(app)  # Enable CORS for all routes

# Path to the test file
TEST_FILE_PATH = './messages.json'

# Ensure the test file exists
def ensure_file_exists():
    if not os.path.exists(TEST_FILE_PATH):
        with open(TEST_FILE_PATH, 'w') as f:
            json.dump({"message": ""}, f)

# GET /message - Return contents of the test file
@app.route('/message', methods=['GET'])
def get_message():
    ensure_file_exists()
    try:
        with open(TEST_FILE_PATH, 'r') as f:
            data = json.load(f)
        return data["message"]
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# POST /message - Accept a message and write to the test file
@app.route('/message', methods=['POST'])
def post_message():
    ensure_file_exists()
    try:
        message = request.form.get('message') or request.json.get('message', '')
        
        with open(TEST_FILE_PATH, 'w') as f:
            json.dump({"message": message}, f)
            
        return jsonify({"success": True, "message": message})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# GET / - Basic form web page
@app.route('/', methods=['GET'])
def index():
    html = '''
    <!DOCTYPE html>
    <html>
    <head>
        <title>Message Form</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                max-width: 600px;
                margin: 0 auto;
                padding: 20px;
            }
            .form-group {
                margin-bottom: 15px;
            }
            label {
                display: block;
                margin-bottom: 5px;
            }
            input[type="text"] {
                width: 100%;
                padding: 8px;
                box-sizing: border-box;
            }
            button {
                background-color: #4CAF50;
                color: white;
                padding: 10px 15px;
                border: none;
                cursor: pointer;
            }
            .message-display {
                margin-top: 20px;
                padding: 10px;
                border: 1px solid #ddd;
                background-color: #f9f9f9;
            }
        </style>
    </head>
    <body>
        <h1>Message Form</h1>
        
        <form id="messageForm">
            <div class="form-group">
                <label for="message">Enter Message:</label>
                <input type="text" id="message" name="message" required>
            </div>
            <button type="submit">Save Message</button>
        </form>
        
        <div class="message-display">
            <h2>Current Message:</h2>
            <p id="currentMessage">Loading...</p>
        </div>
        
        <script>
            // Load the current message when page loads
            window.addEventListener('DOMContentLoaded', async () => {
                await loadMessage();
            });
            
            // Handle form submission
            document.getElementById('messageForm').addEventListener('submit', async (e) => {
                e.preventDefault();
                
                const message = document.getElementById('message').value;
                
                try {
                    const response = await fetch('/message', {
                        method: 'POST',
                        headers: {
                            'Content-Type': 'application/json'
                        },
                        body: JSON.stringify({ message })
                    });
                    
                    if (response.ok) {
                        document.getElementById('message').value = '';
                        await loadMessage();
                        alert('Message saved successfully!');
                    } else {
                        alert('Error saving message');
                    }
                } catch (error) {
                    console.error('Error:', error);
                    alert('Error saving message');
                }
            });
            
            // Function to load the current message
            async function loadMessage() {
                try {
                    const response = await fetch('/message');
                    const data = await response.json();
                    
                    document.getElementById('currentMessage').textContent = 
                        data.message ? data.message : 'No message saved yet';
                } catch (error) {
                    console.error('Error:', error);
                    document.getElementById('currentMessage').textContent = 
                        'Error loading message';
                }
            }
        </script>
    </body>
    </html>
    '''
    return html

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3000, debug=True)