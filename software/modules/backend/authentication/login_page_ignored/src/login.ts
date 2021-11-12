import Translator from '@andreasremdt/simple-translator';

export function login() {
    let username = (<HTMLInputElement>document.getElementById("username")).value;
    let password = (<HTMLInputElement>document.getElementById("password")).value;

    let xhr = new XMLHttpRequest();
    let result = false;
    xhr.onreadystatechange = function () {
        if(xhr.readyState === XMLHttpRequest.DONE) {
            let status = xhr.status;
            if (status == 200) {
                 window.location.href = "http://" + location.host;
                 result = true;
            } else if (status == 401) {
                console.log("credentials incorrect?");
            } else {
                console.log("error: got status ");
                console.log(status);
            }
        }
    }
    xhr.open("GET", "http://" + location.host + "/credential_check", false, username, password);
    xhr.send();
    return result;
}

window.onload = () => {
    let de = {
        "username": "Benutzername",
        "password": "Passwort",
        "show_password": "Anzeigen",
        "login": "Anmeldung",
        "login_button": "Anmelden"
    };
    let en = {
        "username": "Username",
        "password": "Password",
        "show_password": "Show",
        "login": "Sign In",
        "login_button": "Sign In"
    };

    var translator = new Translator();

    // Add the language to the translator and translate the page
    translator.add('de', de).add('en', en).translatePageTo();

    window.setTimeout(() => document.getElementById('main').removeAttribute("style"), 200);

    let show_password = <HTMLInputElement>document.getElementById('show_password');
    let password = <HTMLInputElement>document.getElementById('password');
    show_password.addEventListener("change", () => {
        if (password.type == 'password')
            password.type = 'text';
        else
            password.type = 'password';
    });

    let form = <HTMLFormElement>document.getElementById('login_form');
    form.addEventListener('submit', function (event: Event) {
        event.preventDefault();
        event.stopPropagation();

        if(!login()) {
            password.classList.add("shake");
            window.setTimeout(() => {
                password.classList.remove("shake");
                password.focus();
            }, 500);
        }
    }, false);
}
