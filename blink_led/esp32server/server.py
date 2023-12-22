from flask import Flask, jsonify, request, render_template
from flask_sock import Sock
import json
import time
import threading
mutex = threading.Lock()

app = Flask(__name__)

sock = Sock()
sock.init_app(app)

desired_data = {"led":0}
connection_status = {"last_update":0}

# receive data from 
# the web interface
@sock.route('/update')
def update(sock):
    global desired_data
    while True:
        # trigger from led_button
        # to toggle the led
        sock.receive() # blocking receive

        mutex.acquire()
        desired_data["led"] = not connection_status["led"]
        mutex.release()
        
@sock.route('/echo')
def echo(sock):
    global connection_status
    while True:
        mutex.acquire()
        ws_data = json.dumps(connection_status)
        mutex.release()

        sock.send(ws_data)

        time.sleep(.5)

def draw_received_data(connection_status):
    data = "<h2>Esp32 State</h2>"
    data += '<table>'
    for k,v in connection_status.items():
        data += '<tr>'
        data += '<td>{}</td>'.format(k)
        data += '<td id={}>{}</td>'.format(k,v)
    data += "</table>"
    return data

# receives data from the esp32
@app.route('/service',methods=['POST'])
def service():
    global desired_data
    global connection_status

    # received the state data from
    # the esp32
    received_data = json.loads(request.data)

    # save the data into the 
    # connection status dictionary
    # add in a timestamp
    mutex.acquire()
    connection_status["last_update"] = time.time()
    for k,v in received_data.items():
        connection_status[k] = v
    mutex.release()

    # send the desired state information 
    # back to the esp32
    return jsonify(desired_data)

@app.route('/', methods=["GET","POST"])
def index():
    return render_template("home.html", 
                           data=draw_received_data(connection_status))

if __name__ == '__main__':
    app.run(host='0.0.0.0',debug=True)
