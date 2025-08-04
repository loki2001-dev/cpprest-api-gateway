from flask import Flask, jsonify, request

app = Flask(__name__)

orders = []

@app.route('/orders/create', methods=['POST'])
def create_order():
    data = request.json
    if not data or 'product_id' not in data or 'quantity' not in data:
        return jsonify({"error": "Invalid data"}), 400
    order_id = len(orders) + 1
    order = {"id": order_id, "product_id": data['product_id'], "quantity": data['quantity']}
    orders.append(order)
    return jsonify(order), 201

if __name__ == '__main__':
    app.run(port=9003)