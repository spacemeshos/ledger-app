const http = require('http');

const getJsonHttp = async (url) => new Promise((resolve, reject) => {
  const req = http.get(url, (res) => {
    let data = '';
    res.on('data', (chunk) => {
      data += chunk;
    });
    res.on('end', () => {
      try {
        resolve(JSON.parse(data));
      } catch (err) {
        reject(err);
      }
    });
  });
  req.on('error', reject);
  req.end();
});

const getEvents = async (callback) => new Promise((resolve, reject) => {
  const req = http.get('http://127.0.0.1:5000/events?stream=true', (res) => {
    res.on('data', (chunk) => {
      if (chunk.length > 6) {
        try {
          const strings = chunk.toString().split('data: ');
          if (callback) {
            for (const str of strings) {
              if (str.length > 0 && !callback(JSON.parse(str.trimEnd()))) {
                req.destroy();
              }
            }
          }
        } catch (err) {
          reject(err);
        }
      }
    });
    res.on('end', () => {
      resolve('OK');
    });
  });
  req.on('error', reject);
  req.end();
});

const postJsonHttp = async (url, data) => new Promise((resolve, reject) => {
  const req = http.request(url,
    {method: 'POST', headers: {'Content-Type': 'application/json'}}, (res) => {
      let data = '';
      res.on('data', (chunk) => {
        data += chunk;
      });
      res.on('end', () => {
        if (res.statusCode === 200 || res.statusCode === 201) {
          try {
            console.log('Response:', data);
            resolve(JSON.parse(data));
          } catch (err) {
            reject(err);
          }
        } else {
          reject(res.statusCode);
        }
      });
    });
  req.on('error', reject);
  console.log('Request:', data);
  req.write(data);
  req.end();
});

const sendApdu = async (data) => {
  return postJsonHttp('http://127.0.0.1:5000/apdu', '{"data":"' + data + '"}');
}

const pressLeft = async () => {
  return postJsonHttp('http://127.0.0.1:5000/button/left', '{"action":"press-and-release"}');
}

const pressBoth = async () => {
  return postJsonHttp('http://127.0.0.1:5000/button/both', '{"action":"press-and-release"}');
}

const pressRight = async () => {
  return postJsonHttp('http://127.0.0.1:5000/button/right', '{"action":"press-and-release"}');
}

const doTest = async (test) => {
  let step = -1;
  const events = getEvents((data) => {
    console.log('Event:', data);
    if (step === -1) {
      for (let i = 0; i < test.events.length; i++) {
        if (test.events[i].text === data.text) {
          if (test.events[i].skip) {
            return true;
          }
          step = i;
          break;
        }
      }
      if (step === -1) {
        throw new Error('Unexpected event ' + data.text);
      }
    }
    const event = test.events[step];
    if (data.text !== event.text) {
      throw new Error('Unexpected event ' + data.text);
    }
    if (event.action) {
      event.action().catch(err => {
        console.log(err);
        process.exit(1);
      });
    }
    step++;
    return step < test.events.length;
  }).catch(err => {
    console.log(err);
    process.exit(1);
  });

  const response = await sendApdu(test.apdus[0].request);
  if (response.data !== test.apdus[0].response) {
    throw new Error('Response mismatch');
  }

  await events;
}

doTest({
  apdus: [
    {request: '3010000015058000002c8000021c800000000000000080000000', response: 'a47a88814cecde42f2ad0d75123cf530fbe8e5940bbc44273014714df9a33e165b8a438f52626879ca201f8d2e938dad5f1e99c01ae67a8bd4f805e25ab746309000'}
  ],
  events: [
    {text: 'Spacemesh', skip: true},
    {text: 'is ready', skip: true},
    {text: 'Export public key'},
    {text: "m/44'/540'/0'/0/0", action: pressBoth},
    {text: 'Confirm export'},
    {text: 'public key?', action: pressRight},
    {text: 'Spacemesh'},
    {text: 'is ready'}
  ]
}).then(data => process.exit(0)).catch(err => process.exit(1));
