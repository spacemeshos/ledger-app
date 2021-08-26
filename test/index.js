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

  for (const apdu of test.apdus) {
    const response = await sendApdu(apdu.request);
    if (response.data !== apdu.response) {
      throw new Error('Response mismatch');
    }
  }

  await events;
}

const doTests = async () => {
  // Test getExtendedPublicKey
  await doTest({
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
  });

  // Test getAddress
  await doTest({
    apdus: [
      {request: '3011010015058000002c8000021c800000000000000080000000', response: 'a47a88814cecde42f2ad0d75123cf530fbe8e5949000'}
    ],
    events: [
      {text: 'Spacemesh', skip: true},
      {text: 'is ready', skip: true},
      {text: 'Export address'},
      {text: "Path: m/44'/540'/", action: pressBoth},
      {text: 'Confirm'},
      {text: 'export address?', action: pressRight},
      {text: 'Spacemesh'},
      {text: 'is ready'}
    ]
  });

  // Test showAddress
  await doTest({
    apdus: [
      {request: '3011020015058000002c8000021c800000000000000080000000', response: '9000'}
    ],
    events: [
      {text: 'Spacemesh', skip: true},
      {text: 'is ready', skip: true},
      {text: 'Verify address'},
      {text: 'Make sure it agre', action: pressBoth},
      {text: 'Address path'},
      {text: "m/44'/540'/0'/0/0", action: pressBoth},
      {text: 'Address'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Spacemesh'},
      {text: 'is ready'}
    ]
  });

  // Test signCoinTx
  await doTest({
    apdus: [{
      request: '302005008a058000002c8000021c8000000000000000800000001835df3489b3a39e0f38a77d347f8327e8937c623543b84bd8734fc237ae3f33000000000000000001a47a88814cecde42f2ad0d75123cf530fbe8e59400000000000f424000000000000003e8000000e8d4a510008eb640bb3c0b63923637e07e4bc82ea032e2cceec59ada068dd5849d971bd099',
      response: '00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a47a88814cecde42f2ad0d75123cf530fbe8e5940bbc44273014714df9a33e169000'
    }],
    events: [
      {text: 'Spacemesh', skip: true},
      {text: 'is ready', skip: true},
      {text: 'Tx type:'},
      {text: 'COIN ED', action: pressBoth},
      {text: 'Send SMH'},
      {text: '1.0', action: pressBoth},
      {text: 'To address'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Max Tx Fee'},
      {text: '0.001', action: pressBoth},
      {text: 'Confirm'},
      {text: 'transaction?', action: pressRight},
      {text: 'Signer'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Sign using'},
      {text: 'this signer?', action: pressRight},
      {text: 'Spacemesh'},
      {text: 'is ready'}
    ]
  });

  // Test signAppTx
  await doTest({
    apdus: [{
      request: '30200300f0058000002c8000021c8000000000000000800000001835df3489b3a39e0f38a77d347f8327e8937c623543b84bd8734fc237ae3f33020000000000000001a47a88814cecde42f2ad0d75123cf530fbe8e59400000000000f424000000000000003e8000000e8d4a51000f27596fadee9fd74a7745ab45d978e82e275e34d06e86f127bb1f18bfb8f188994b739420fe48a6fc6aa26e73e79ae743addb37c615c85bfd3688995be7ba7b1ca459caa9b121df4a11125428e186ab633483f01ee14c7f70229153e39a873c6c1a74d00ac388471cd99c6691b0168ee801028b393b66dc88b304e9c179617b213d834d17081',
      response: '9000'
    }, {
      request: '30200200f0701e7e47e25b8c7dbea6378603fb8d94e0912acb8d06692ffdc784a5219d52980f459b23b89ec975bd432acd041415edddd65f19ff5d7bfbb57a72cac44b71f2a8728cb3f460c012592b5a2b7bb4530392498149e9fa70ada5fc1881f6fee14a24eae3caab13c0dbafb9b544bf0f9949654445fa87158ff426424c6fcfc41e314c71bf7c372eaae41079e9f4b27d6e6bb61c5e1e22a7111eb54eaa40133add17d2b9aef11781f65ab8fff5b1649cfd291f828384efa1c89584bd0e71041e3750cf03daa482bda9106cb6f4112b2f4034279bd6abd6e54fedbbafd101ff66ef173d83769cf581e72853dd5cdb42f296f3',
      response: '9000'
    }, {
      request: '30200200f030f66df97100804386cd6bf953ce0cd6e84a388f36be9ceee72d6bcc98fbfc9cfbc5b8f93ab7be5beed22ba72e494667f5548323398156377d930b1efd27971608dd29df8d1523b9ac41a4e87c684ec7ee7790ca5899745b4be54026b3be233a39a454eff328c71995d420a460255a11e4508572915a8c394c764682392a1d1ecf43951119cf41ce582900918629e3b96b34b82396058659931d1904ef8e38cc84e717057d68e37bba33e1f80e36a9031e39e84b6523f0470ffa41515f8847f5a0e7cd8cc31bf287639a4d0249a50d866f6400c95f7f895f16ced4148f1b4c1ba2b63a7b4423adf960850432dea6b22b',
      response: '9000'
    }, {
      request: '30200200f0ab7018bc20713348dbca2722a75d08f9ab60718355ba7063d3373cd547a3a019b78ae83e6cf5fc9bcc3567ccff0d298a98cf2fa66770007428b64645f110521e83257bbbb686d23a89e36b92eeee61334d2ca644489478fb48b1afd08737fcaacf637b9425ee514480aafe0af536fed8361a09503ccf512456c82cdf11e92f7d2e032a3c622cd113f1ea75f5ce1a6d93b67bfdb6354cf8496158efbbe5b97f074e2c062a965bd44569040da5056a02ad03eeb9bd42c8fdae963aba569c7643fe2fbe106f5e3cce03e5bd773074e24dd26298f07f8a5a1b00b31dcbaf0de81f0a2d7234d1ddd234d2093d66a515eb4c0e',
      response: '9000'
    }, {
      request: '30200400ca0c5e144e0cac300a151f81a2f9a4c87dbd1b8a602ae80fba55913a0830c0d7f34a5fed47a5da7c776bb159ab8c2aa492f4c06c8519717ac6abc4413efaa8ff0e569768441a3780abb21c71d81d9222c6d225d87744a60433c5741dd6fbf363fab3a93cabbf8b16c6ef580fc359fc5e7853bf24a7825f8aba7a1497a2d4b892ccd2d1ffa37ffcf4b4cc872b0170bde7f2bdefc226dd50d2aaf3f5a971593a0d52ee0c5979e233ab08dbf88eb640bb3c0b63923637e07e4bc82ea032e2cceec59ada068dd5849d971bd099',
      response: '00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a47a88814cecde42f2ad0d75123cf530fbe8e5940bbc44273014714df9a33e169000'
    }],
    events: [
      {text: 'Spacemesh', skip: true},
      {text: 'is ready', skip: true},
      {text: 'Tx type:'},
      {text: 'EXEC APP ED', action: pressBoth},
      {text: 'Send SMH'},
      {text: '1.0', action: pressBoth},
      {text: 'To address'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Max Tx Fee'},
      {text: '0.001', action: pressBoth},
      {text: 'Confirm'},
      {text: 'transaction?', action: pressRight},
      {text: 'Signer'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Sign using'},
      {text: 'this signer?', action: pressRight},
      {text: 'Spacemesh'},
      {text: 'is ready'}
    ]
  });

  // Test signSpawnTx
  await doTest({
    apdus: [{
      request: '30200300f0058000002c8000021c8000000000000000800000001835df3489b3a39e0f38a77d347f8327e8937c623543b84bd8734fc237ae3f33040000000000000001a47a88814cecde42f2ad0d75123cf530fbe8e59400000000000f424000000000000003e8000000e8d4a51000f27596fadee9fd74a7745ab45d978e82e275e34d06e86f127bb1f18bfb8f188994b739420fe48a6fc6aa26e73e79ae743addb37c615c85bfd3688995be7ba7b1ca459caa9b121df4a11125428e186ab633483f01ee14c7f70229153e39a873c6c1a74d00ac388471cd99c6691b0168ee801028b393b66dc88b304e9c179617b213d834d17081',
      response: '9000'
    }, {
      request: '30200200f0701e7e47e25b8c7dbea6378603fb8d94e0912acb8d06692ffdc784a5219d52980f459b23b89ec975bd432acd041415edddd65f19ff5d7bfbb57a72cac44b71f2a8728cb3f460c012592b5a2b7bb4530392498149e9fa70ada5fc1881f6fee14a24eae3caab13c0dbafb9b544bf0f9949654445fa87158ff426424c6fcfc41e314c71bf7c372eaae41079e9f4b27d6e6bb61c5e1e22a7111eb54eaa40133add17d2b9aef11781f65ab8fff5b1649cfd291f828384efa1c89584bd0e71041e3750cf03daa482bda9106cb6f4112b2f4034279bd6abd6e54fedbbafd101ff66ef173d83769cf581e72853dd5cdb42f296f3',
      response: '9000'
    }, {
      request: '30200200f030f66df97100804386cd6bf953ce0cd6e84a388f36be9ceee72d6bcc98fbfc9cfbc5b8f93ab7be5beed22ba72e494667f5548323398156377d930b1efd27971608dd29df8d1523b9ac41a4e87c684ec7ee7790ca5899745b4be54026b3be233a39a454eff328c71995d420a460255a11e4508572915a8c394c764682392a1d1ecf43951119cf41ce582900918629e3b96b34b82396058659931d1904ef8e38cc84e717057d68e37bba33e1f80e36a9031e39e84b6523f0470ffa41515f8847f5a0e7cd8cc31bf287639a4d0249a50d866f6400c95f7f895f16ced4148f1b4c1ba2b63a7b4423adf960850432dea6b22b',
      response: '9000'
    }, {
      request: '30200200f0ab7018bc20713348dbca2722a75d08f9ab60718355ba7063d3373cd547a3a019b78ae83e6cf5fc9bcc3567ccff0d298a98cf2fa66770007428b64645f110521e83257bbbb686d23a89e36b92eeee61334d2ca644489478fb48b1afd08737fcaacf637b9425ee514480aafe0af536fed8361a09503ccf512456c82cdf11e92f7d2e032a3c622cd113f1ea75f5ce1a6d93b67bfdb6354cf8496158efbbe5b97f074e2c062a965bd44569040da5056a02ad03eeb9bd42c8fdae963aba569c7643fe2fbe106f5e3cce03e5bd773074e24dd26298f07f8a5a1b00b31dcbaf0de81f0a2d7234d1ddd234d2093d66a515eb4c0e',
      response: '9000'
    }, {
      request: '30200400ca0c5e144e0cac300a151f81a2f9a4c87dbd1b8a602ae80fba55913a0830c0d7f34a5fed47a5da7c776bb159ab8c2aa492f4c06c8519717ac6abc4413efaa8ff0e569768441a3780abb21c71d81d9222c6d225d87744a60433c5741dd6fbf363fab3a93cabbf8b16c6ef580fc359fc5e7853bf24a7825f8aba7a1497a2d4b892ccd2d1ffa37ffcf4b4cc872b0170bde7f2bdefc226dd50d2aaf3f5a971593a0d52ee0c5979e233ab08dbf88eb640bb3c0b63923637e07e4bc82ea032e2cceec59ada068dd5849d971bd099',
      response: '00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a47a88814cecde42f2ad0d75123cf530fbe8e5940bbc44273014714df9a33e169000'
    }],
    events: [
      {text: 'Spacemesh', skip: true},
      {text: 'is ready', skip: true},
      {text: 'Tx type:'},
      {text: 'SPAWN APP ED', action: pressBoth},
      {text: 'Send SMH'},
      {text: '1.0', action: pressBoth},
      {text: 'To address'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Max Tx Fee'},
      {text: '0.001', action: pressBoth},
      {text: 'Confirm'},
      {text: 'transaction?', action: pressRight},
      {text: 'Signer'},
      {text: 'a47a88814cecde42f', action: pressBoth},
      {text: 'Sign using'},
      {text: 'this signer?', action: pressRight},
      {text: 'Spacemesh'},
      {text: 'is ready'}
    ]
  });
}

doTests().then(data => process.exit(0)).catch(err => process.exit(1));
