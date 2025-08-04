import logging
from flask import Flask, jsonify, request

app = Flask(__name__)

logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s %(levelname)s %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S')

@app.before_request
def log_request_info():
    app.logger.info(f"Request: {request.method} {request.path} from {request.remote_addr}")

users = [
    {"id": 1, "name": "Tyson"},
    {"id": 2, "name": "Rocky"},
]

@app.route('/users', methods=['GET'])
def get_users():
    return jsonify(users)

@app.route('/users/create', methods=['POST'])
def create_user():
    data = request.json
    if not data or 'name' not in data:
        return jsonify({"error": "Invalid data"}), 400
    new_id = max(u["id"] for u in users) + 1 if users else 1
    new_user = {"id": new_id, "name": data['name']}
    users.append(new_user)
    return jsonify(new_user), 201

@app.route('/users/<int:user_id>', methods=['GET'])
def get_user(user_id):
    user = next((u for u in users if u["id"] == user_id), None)
    if user:
        return jsonify(user)
    else:
        return jsonify({"error": "User not found"}), 404

@app.route('/users/<int:user_id>/update', methods=['PUT'])
def update_user(user_id):
    user = next((u for u in users if u["id"] == user_id), None)
    if not user:
        return jsonify({"error": "User not found"}), 404
    data = request.json
    if not data or 'name' not in data:
        return jsonify({"error": "Invalid data"}), 400
    user['name'] = data['name']
    return jsonify(user)

@app.route('/users/<int:user_id>/delete', methods=['DELETE'])
def delete_user(user_id):
    global users
    users = [u for u in users if u["id"] != user_id]
    return jsonify({"message": "User deleted"})

if __name__ == '__main__':
    # debug=True 옵션은 개발용, 더 자세한 로그와 에러페이지 제공
    app.run(port=9001, debug=True)