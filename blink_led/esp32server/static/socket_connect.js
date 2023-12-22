
const connection_data_socket = new WebSocket('ws://bilbo:5000/echo');
connection_data_socket.addEventListener('message',  ev => {

    // parse the connection_data json 
    // from flask into a JS struct
    var data = JSON.parse(ev.data); 

    // update timestamp field in the table
    last_update.innerHTML = data.last_update;

    // update current led state in the table
    led.innerHTML = data.led

    //console.log(data.led)

    // update led button label from current LED state
    if(  data.led == 1 )
    {
        led_button.innerHTML = "ON";
        led_button.style.background = '#adff2f'
        led_button.style.color = 'black'
    }
    else
    {
        led_button.innerHTML = "OFF";
        led_button.style.background = '#0b430b'
        led_button.style.color = 'white'
    }
});

const socket_update = new WebSocket('ws://bilbo:5000/update');
led_button.onclick = ev => {
    socket_update.send('1');
};
