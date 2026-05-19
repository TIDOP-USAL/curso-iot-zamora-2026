// TTN Payload decoder. Curso IoT Zamora 2026
function decodeUplink(input) {

  let temperature_bytes = (new Uint8Array([
    input.bytes[0],
    input.bytes[1]
    ])).buffer;
  let temperature = (new DataView(temperature_bytes)).getInt16(0) / 100.0;

  let pressure_bytes = (new Uint8Array([
    input.bytes[2],
    input.bytes[3],
    input.bytes[4],
    input.bytes[5]
    ])).buffer;
  let pressure = (new DataView(pressure_bytes)).getUint32(0) / 100.0;

  return {
    data: {
      temperature,
      pressure
    },
    warnings: [],
    errors: []
  };
}
