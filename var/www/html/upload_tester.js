const uploadApiBase = "http://127.0.0.1:8081"; // conf/webserv.conf: POST + DELETE
const listApiUrl = "http://127.0.0.1:8080/upload/"; // conf/webserv.conf: GET location /upload

const uploadForm = document.getElementById("upload-form");
const deleteForm = document.getElementById("delete-form");
const fileInput = document.getElementById("file-input");
const targetNameInput = document.getElementById("target-name");
const deleteNameInput = document.getElementById("delete-name");
const fileList = document.getElementById("file-list");
const listNote = document.getElementById("list-note");
const refreshButton = document.getElementById("refresh-list");
const logBox = document.getElementById("log");

const localCache = new Set();

function now() {
	return new Date().toLocaleTimeString();
}

function log(message) {
	logBox.textContent = `[${now()}] ${message}\n` + logBox.textContent;
}

function normalizeName(name) {
	return name.trim().replace(/^\/+/, "");
}

function renderList(names) {
	fileList.innerHTML = "";
	if (!names.length) {
		const li = document.createElement("li");
		li.textContent = "No file detected.";
		fileList.appendChild(li);
		return;
	}

	names.sort().forEach((name) => {
		const li = document.createElement("li");
		const span = document.createElement("span");
		span.textContent = name;

		const actions = document.createElement("div");
		const delBtn = document.createElement("button");
		delBtn.className = "danger";
		delBtn.textContent = "DELETE";
		delBtn.addEventListener("click", () => requestDelete(name));

		actions.appendChild(delBtn);
		li.appendChild(span);
		li.appendChild(actions);
		fileList.appendChild(li);
	});
}

async function requestUpload(file, targetName) {
	const finalName = normalizeName(targetName || file.name);
	if (!finalName) {
		log("Upload aborted: invalid filename.");
		return;
	}

	const form = new FormData();
	form.append("file", file, finalName);

	const url = `${uploadApiBase}/${encodeURIComponent(finalName)}`;
	log(`POST ${url}`);

	const response = await fetch(url, {
		method: "POST",
		body: form
	});

	log(`POST response: ${response.status} ${response.statusText}`);
	if (response.ok || response.status === 201) {
		localCache.add(finalName);
		await refreshFiles();
	}
}

async function requestDelete(fileName) {
	const finalName = normalizeName(fileName);
	if (!finalName) {
		log("Delete aborted: invalid filename.");
		return;
	}

	const url = `${uploadApiBase}/${encodeURIComponent(finalName)}`;
	log(`DELETE ${url}`);

	const response = await fetch(url, {
		method: "DELETE"
	});

	log(`DELETE response: ${response.status} ${response.statusText}`);
	if (response.ok) {
		localCache.delete(finalName);
		await refreshFiles();
	}
}

async function fetchRemoteList() {
	const response = await fetch(listApiUrl, {
		method: "GET"
	});
	if (!response.ok) {
		throw new Error(`List endpoint answered with ${response.status}`);
	}

	const html = await response.text();
	const doc = new DOMParser().parseFromString(html, "text/html");
	const links = Array.from(doc.querySelectorAll("a"));

	const names = links
		.map((a) => (a.textContent || "").trim().replace(/\/$/, ""))
		.filter((name) => name && name !== "." && name !== "..");

	return [...new Set(names)];
}

async function refreshFiles() {
	try {
		const remote = await fetchRemoteList();
		listNote.textContent = "List source: GET http://127.0.0.1:8080/upload/";
		renderList(remote);
	} catch (error) {
		const fallback = Array.from(localCache);
		listNote.textContent = "Remote listing unavailable (likely autoindex/cors). Showing local upload cache only.";
		renderList(fallback);
		log(`List warning: ${error.message}`);
	}
}

uploadForm.addEventListener("submit", async (event) => {
	event.preventDefault();
	if (!fileInput.files || !fileInput.files[0]) {
		log("No file selected.");
		return;
	}
	try {
		await requestUpload(fileInput.files[0], targetNameInput.value);
		uploadForm.reset();
	} catch (error) {
		log(`POST error: ${error.message}`);
	}
});

deleteForm.addEventListener("submit", async (event) => {
	event.preventDefault();
	try {
		await requestDelete(deleteNameInput.value);
		deleteForm.reset();
	} catch (error) {
		log(`DELETE error: ${error.message}`);
	}
});

refreshButton.addEventListener("click", refreshFiles);

refreshFiles();
