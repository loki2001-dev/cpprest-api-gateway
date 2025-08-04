from flask import Flask, jsonify

app = Flask(__name__)

products = [
    {"id": 1, "name": "Boxing gloves"},
    {"id": 2, "name": "Training shoes"},
]

@app.route('/products', methods=['GET'])
def get_products():
    return jsonify(products)

if __name__ == '__main__':
    app.run(port=9002)