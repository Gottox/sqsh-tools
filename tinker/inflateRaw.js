let  { inflateRaw, unzip } = require('zlib');

async function streamToBuffer (stream) {
	return new Promise((resolve, reject) => {
		const data = [];

		stream.on('data', (chunk) => {
			data.push(chunk);
		});

		stream.on('end', () => {
			resolve(Buffer.concat(data))
		})

		stream.on('error', (err) => {
			reject(err)
		})
	})
}

(async () => {
	const buffer = await streamToBuffer(process.stdin);
	inflateRaw(buffer, (err, buf) => {
		process.stdout.write(buf);
	});
})()
