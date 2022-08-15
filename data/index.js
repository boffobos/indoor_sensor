var button = document.querySelector('#submit-config');
var form = document.querySelector('form');
var room = document.querySelector('#room');
var ssid = document.querySelector('#ssid');
var pass = document.querySelector('#password');
var email = document.querySelector('#email');
var servIp = document.querySelector('#altServerIP');

function notify(message, type) {
    const notifyField = document.querySelector('.notification');
    const types = {
        error: '#ff0000',
        success: '#84f375',
        normal: '#3f3f3f'
    }
    let isTypeExist = false;
    notifyField.textContent = message;

    for (const key in types) {
        if (key == type) isTypeExist = true;
    }

    if (isTypeExist) {
        notifyField.style.color = types[type];
    } else {
        notifyField.style.color = types['normal'];
    }

    notifyField.style.display = 'block';
    return setTimeout(() => {
        notifyField.style.display = 'none';
    }, 3000)
}

function setErrorField(body) {
    let haveErrors = false;
    for (const field in body) {
        const isInputRequired = document.querySelector(`.${field} input`).required
        const errorField = document.querySelector(`.${field} .error`);
        if (!body[`${field}`] && isInputRequired) {
            errorField.textContent = 'Required';
            errorField.style.display = 'block';
            haveErrors = true
        } else {
            errorField.textContent = '';
            errorField.style.display = 'none';
        }

    }
    return haveErrors;
}


button.addEventListener('click', (e) => {
    e.preventDefault();
    let body = {
        room: room.value || null,
        ssid: ssid.value || null,
        password: pass.value || null,
        email: email.value || null,
        altServerIp: servIp.value || null
    }

    if (setErrorField(body)) return;

    body = JSON.stringify(body);

    const fetchOptions = {
        method: 'POST',
        headers: {
            'Content-type': 'application/json'
        },
        body,
        mode: 'no-cors'
    }
    fetch('/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: body,
    })
        .then(res => {
            if (res.status == 200) {
                notify('Data sent successfuly', 'success');
            }
        });

});