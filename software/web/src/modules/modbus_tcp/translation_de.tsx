/** @jsxImportSource preact */
import { h } from "preact";
import * as options from "../../options";
import { removeUnicodeHacks } from "../../ts/translation";
let x = {
    "modbus_tcp": {
        "navbar": {
            "modbus_tcp": "Modbus/TCP"
        },
        "content": {
            "disabled": "Deaktiviert",
            "read_only": "Nur Lesezugriff (Fehler bei Schreibzugriff)",
            "ignore_writes": "Nur Lesezugriff (Schreibzugriffe ignorieren)",
            "full_access": "Lese-/Schreibzugriff",
            "table": "Registertabelle",
            "warp": null,
            "bender_emulate": "Open Modbus Charge Control Interface (OMCCI); Kompatibel zu Bender CC613",
            "keba_emulate": "Kompatibel zu Keba P30 C-Series",
            "modbus_tcp": "Modbus/TCP",
            "enable": "Modbus/TCP-Server-Modus",
            "port": "Port",
            "port_muted": "typischerweise 502",
            "debug": "Debug-Client",
            "table_docu": "Register-Dokumentation",
            "table_content": <>
                <thead class="thead-light">
                    <tr>
                        <th scope="col">Registeradresse</th>
                        <th scope="col">Name</th>
                        <th scope="col">Typ</th>
                        <th scope="col">Benötigtes Feature</th>
                        <th scope="col">Beschreibung</th>
                    </tr>
                </thead>
                <thead>
                    <tr>
                        <th colSpan={5}>Input Registers (Funktionscodes: Lesen: 4)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Version der Registertabelle</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Aktuelle Version: 4</td>
                    </tr>
                    <tr>
                        <td>2</td>
                        <td>Firmware-Version Major</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Beispielsweise 2 für Firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>4</td>
                        <td>Firmware-Version Minor</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Beispielsweise 4 für Firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>6</td>
                        <td>Firmware-Version Patch</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Beispielsweise 0 für Firmware 2.4.0+66558ade</td>
                    </tr>
                    <tr>
                        <td>8</td>
                        <td>Zeitstempel der Firmware</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Unix-Timestamp des Zeitpunktes an dem eine Firmware gebaut wurde. Beispielsweise 0x66558ADE für
                            Firmware 2.4.0+66558ade (in Dezimal 1716882142), entspricht dem 28. Mai 2024 07:42:22 UTC.</td>
                    </tr>
                    <tr>
                        <td>10</td>
                        <td>Charger-ID</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Dekodierte Form der Base58-UID, die für Standard-Hostnamen, -SSID usw. genutzt wird. Zum Beispiel
                            185460 für X8A.</td>
                    </tr>
                    <tr>
                        <td>12</td>
                        <td>Uptime (s)</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Zeit in Sekunden seit dem Start der Wallbox-Firmware.</td>
                    </tr>
                    <tr>
                        <td>1000</td>
                        <td>IEC-61851-Zustand</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            <ul>
                                <li>0: A</li>
                                <li>1: B</li>
                                <li>2: C</li>
                                <li>3: D</li>
                                <li>4: E/F</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1002</td>
                        <td>Ladestatus</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            <ul>
                                <li>0: Nicht verbunden</li>
                                <li>1: Warte auf Freigabe</li>
                                <li>2: Ladebereit</li>
                                <li>3: Lädt</li>
                                <li>4: Fehler</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1004</td>
                        <td>User-ID</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>ID des Benutzers der den Ladevorgang gestartet hat. 0 falls eine Freigabe ohne Benutzerzuordnung erfolgt
                            ist. 0xFFFFFFFF falls gerade kein Ladevorgang läuft.</td>
                    </tr>
                    <tr>
                        <td>1006</td>
                        <td>Start-Zeitstempel (min)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Ein Unix-Timestamp in <strong>Minuten</strong>, der den Startzeitpunkt des Ladevorgangs angibt. 0
                            falls zum Startzeitpunkt keine Zeitsynchronisierung verfügbar war.</td>
                    </tr>
                    <tr>
                        <td>1008</td>
                        <td>Ladedauer (s)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Dauer des laufenden Ladevorgangs in Sekunden. Auch ohne Zeitsynchronisierung verfügbar.</td>
                    </tr>
                    <tr>
                        <td>1010</td>
                        <td>Erlaubter Ladestrom</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Maximal erlaubter Ladestrom, der dem Fahrzeug zur Verfügung gestellt wird. Dieser Strom ist das Minimum
                            der Stromgrenzen aller Ladeslots.</td>
                    </tr>
                    <tr>
                        <td>1012 bis 1050</td>
                        <td>Ladestromgrenzen (mA)</td>
                        <td>uint32 (20x)</td>
                        <td>evse</td>
                        <td>Aktueller Wert der Ladestromgrenzen in Milliampere. 0xFFFFFFFF falls eine Stromgrenze nicht aktiv
                            ist. 0 falls eine Stromgrenze blockiert. Sonst zwischen 6000 (6A) und 32000 (32A).</td>
                    </tr>
                    <tr>
                        <td>1100</td>
                        <td>"Taster gedrückt"-Zeitstempel</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Zeit zu der zuletzt der Taster gedrückt wurde. 0 falls der Taster seit dem Start des Ladecontrollers nicht betätigt wurde. Um auf einen Tasterdruck zu reagieren kann dieser Wert periodisch gelesen werden. Bei jeder Änderung des Wertes wurde der Taster mindestens einmal gedrückt.</td>
                    </tr>
                    <tr>
                        <td>1102</td>
                        <td>"Taster losgelassen"-Zeitstempel</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Zeit zu der zuletzt der Taster losgelassen wurde wurde. 0 falls der Taster seit dem Start des Ladecontrollers nicht betätigt wurde. Um auf einen Tasterdruck zu reagieren kann dieser Wert periodisch gelesen werden. Bei jeder Änderung des Wertes wurde der Taster mindestens einmal losgelassen.</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Stromzählertyp</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>
                            <ul>
                                <li>0: Kein Stromzähler verfügbar</li>
                                <li>1: SDM72{options.PRODUCT_ID_IS_WARP_ANY ? <span> (nur WARP1)</span> : undefined}</li>
                                <li>2: SDM630</li>
                                <li>3: SDM72 V2</li>
                                <li>4: SDM72CTM</li>
                                <li>5: SDM630MCT V2</li>
                                <li>6: DSZ15DZMOD</li>
                                <li>7: DEM4A</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>2002</td>
                        <td>Ladeleistung (W)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>Die aktuelle Ladeleistung in Watt.</td>
                    </tr>
                    <tr>
                        <td>2004</td>
                        <td>absolute Energie (kWh)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>Die geladene Energie seit der Herstellung des Stromzählers.</td>
                    </tr>
                    <tr>
                        <td>2006</td>
                        <td>relative Energie (kWh)</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>Die geladene Energie seit dem letzten Reset (siehe Holding Register 2000).</td>
                    </tr>
                    <tr>
                        <td>2008</td>
                        <td>Energie des Ladevorgangs</td>
                        <td>float32</td>
                        <td>meter</td>
                        <td>Die während des laufenden Ladevorgangs geladene Energie.</td>
                    </tr>
                    <tr>
                        <td>2100 bis 2268</td>
                        <td>weitere Stromzähler-Werte</td>
                        <td>float32 (85x)</td>
                        <td>all_values</td>
                        <td>{options.WARP_DOC_BASE_URL.length > 0 ? <span>Siehe <a href={removeUnicodeHacks(`${options.WARP_DOC_BASE_URL}/docs/mqtt_http/api_reference/meter/#meter_all_values_any`)}>API-Dokumentation</a></span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>3100</td>
                        <td>Verbundene Phasen</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>Gibt an wie viele Phasen mit dem Fahrzeug verbunden sind (1 oder 3).</td>
                    </tr>
                    <tr>
                        <td>3102</td>
                        <td>Zustand der Phasenumschaltung</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>
                            Aktueller Zustand der Phasenumschaltung:
                            <ul>
                                <li>0: Phasenumschaltung ist bereit für Kommandos.</li>
                                <li>1: Phasenumschaltung ist über die Einstellungen deaktiviert.</li>
                                <li>2: Phasenumschaltung ist aktiviert aber aktuell nicht verfügbar.</li>
                                <li>3: Phasenumschaltung wird gerade durchgeführt; ankommende Kommandos werden ignoriert.</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>4000 bis 4009</td>
                        <td>ID des letzten NFC-Tags</td>
                        <td>uint8 (20x)</td>
                        <td>nfc</td>
                        <td>Die ID des zuletzt erkannten NFC-Tags als ASCII-kodierter Hex-String.</td>
                    </tr>
                    <tr>
                        <td>4010</td>
                        <td>Alter des letzten NFC-Tags</td>
                        <td>uint32</td>
                        <td>nfc</td>
                        <td>Zeit in Millisekunden seitdem das zuletzt erkannten NFC-Tag das letzte mal erkannt wurde. Zeiten &lt;
                            1000 ms bedeuten typischerweise, dass das Tag gerade an die Wallbox gehalten wird.</td>
                    </tr>
                    <tr>
                        <td>4012 bis 4013</td>
                        <td>Typ des letzten NFC-Tags</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>
                            Typ des zuletzt erkannten NFC-Tags als ASCII-kodierter Hex-String.
                            <ul>
                                <li>"0000": Mifare Classic</li>
                                <li>"0001": NFC Forum Typ 1</li>
                                <li>"0002": NFC Forum Typ 2</li>
                                <li>"0003": NFC Forum Typ 3</li>
                                <li>"0004": NFC Forum Typ 4</li>
                                <li>"0005": NFC Forum Typ 5</li>
                            </ul>
                        </td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Holding Registers (Funktionscodes: Lesen: 3, Schreiben: 16)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Reboot</td>
                        <td>uint32</td>
                        <td>---</td>
                        <td>Startet die Wallbox (bzw. den ESP-Brick) neu, um beispielsweise Konfigurationsänderungen
                            anzuwenden. Passwort: 0x012EB007</td>
                    </tr>
                    <tr>
                        <td>1000</td>
                        <td>Ladefreigabe</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td><strong>Veraltet. Stattdessen Coil 1000 verwenden!</strong> 0 zum Blockieren des Ladevorgangs; ein
                            Wert ungleich 0 zum Freigeben.</td>
                    </tr>
                    <tr>
                        <td>1002</td>
                        <td>Erlaubter Ladestrom (mA)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>0 mA oder 6000 mA bis 32000 mA. Andere Ladestromgrenzen können den Strom weiter verringern!</td>
                    </tr>
                    <tr>
                        <td>1004</td>
                        <td>Front-LED-Blinkmuster</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>
                            Steuert die LED des Tasters in der Wallbox-Front. <strong>Blinkmuster und -dauer müssen mit einem Modbus-Befehl geschrieben
                            werden! Falls auch die Farbe gesetzt werden soll{options.PRODUCT_ID_IS_WARP_ANY ? <span> (nur WARP3)</span> : undefined},
                            müssen die Register 1004 bis einschließlich 1013 in einem Befehl geschrieben werden.</strong> Damit die LED kontrolliert
                            werden kann muss die Option "Status-LED-Steuerung" aktiviert sein.

                            <ul>
                                <li>0xFFFFFFFF: EVSE kontrolliert LED</li>
                                <li>0: LED aus</li>
                                <li>1 bis 254: LED gedimmt</li>
                                <li>255: LED an</li>
                                <li>1001: bekanntes NFC-Tag erkannt</li>
                                <li>1002: unbekanntes NFC-Tag erkannt</li>
                                <li>1003: NFC-Tag notwendig</li>
                                <li>2001 bis 2010: Fehlerblinken 1 bis 10 mal</li>
                            </ul>
                        </td>
                    </tr>
                    <tr>
                        <td>1006</td>
                        <td>Front-LED-Blinkdauer</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Die Dauer in Millisekunden für die das in Register 1004 gesetzte Blinkmuster angezeigt werden soll.
                            Maximal 65535 ms werden unterstützt.</td>
                    </tr>
                    <tr>
                        <td>1008</td>
                        <td>Front-LED-Blinkfarbwert (Hue)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Der Farbwert der Farbe (im <a href="https://de.wikipedia.org/wiki/HSV-Farbraum">HSV-Farbraum</a>) in der das im Register 1004 gesetzte Blinkmuster angezeigt werden soll. Nur Werte zwischen 0 und 359 (°) sind erlaubt.{options.PRODUCT_ID_IS_WARP_ANY ? <span> Die Farbe kann nur bei einem WARP3 Charger gesetzt werden. WARP und WARP2 Charger besitzen eine einfarbig blaue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>1010</td>
                        <td>Front-LED-Blinkfarbsättigung (Saturation)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Die Farbsättigung der Farbe (im <a href="https://de.wikipedia.org/wiki/HSV-Farbraum">HSV-Farbraum</a>) in der das im Register 1004 gesetzte Blinkmuster angezeigt werden soll. Nur Werte zwischen 0 und 255 sind erlaubt.{options.PRODUCT_ID_IS_WARP_ANY ? <span> Die Farbe kann nur bei einem WARP3 Charger gesetzt werden. WARP und WARP2 Charger besitzen eine einfarbig blaue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>1012</td>
                        <td>Front-LED-Blinkfarbhelligkeit (Value)</td>
                        <td>uint32</td>
                        <td>evse</td>
                        <td>Die Helligkeit der Farbe (im <a href="https://de.wikipedia.org/wiki/HSV-Farbraum">HSV-Farbraum</a>) in der das im Register 1004 gesetzte Blinkmuster angezeigt werden soll. Nur Werte zwischen 0 und 255 sind erlaubt.{options.PRODUCT_ID_IS_WARP_ANY ? <span> Die Farbe kann nur bei einem WARP3 Charger gesetzt werden. WARP und WARP2 Charger besitzen eine einfarbig blaue LED.</span> : undefined}</td>
                    </tr>
                    <tr>
                        <td>2000</td>
                        <td>Relative Energie zurücksetzen</td>
                        <td>uint32</td>
                        <td>meter</td>
                        <td>Setzt den relativen Energiewert zurück (Input Register 2006). Passwort: 0x3E12E5E7</td>
                    </tr>
                    <tr>
                        <td>3100</td>
                        <td>Phasenumschaltung auslösen</td>
                        <td>uint32</td>
                        <td>phase_switch</td>
                        <td>1 für einphasiges Laden. 3 für dreiphasiges Laden.</td>
                    </tr>
                    <tr>
                        <td>4000 bis 4009</td>
                        <td>ID des vorzutäuschenden NFC-Tags</td>
                        <td>uint8 (20x)</td>
                        <td>nfc</td>
                        <td>
                            Mit den Registern 4000 bis einschließlich 4013 kann ein NFC-Tag vorgetäuscht werden (analog zur API nfc/inject_tag):
                            <ul>
                                <li>Register 4000 bis 4009: Die ID des Tags als ASCII-kodierter Hex-String.</li>
                                <li>
                                    Register 4010 und 4011:
                                    <ul>
                                        <li>"0001": Mit dem vorgetäuschten Tag kann ein Ladevorgang nur gestartet werden (analog zur API nfc/inject_tag_start)</li>
                                        <li>"0002": Mit dem vorgetäuschten Tag kann ein Ladevorgang nur gestoppt werden (analog zur API nfc/inject_tag_stop)</li>
                                        <li>alle anderen Werte: Mit dem vorgetäuschten Tag kann ein Ladevorgang gestartet und gestoppt werden (analog zur API nfc/inject_tag)</li>
                                    </ul>
                                </li>
                                <li>
                                    Register 4012 und 4013: Der Typ des NFC-Tags als ASCII-kodierter Hex-String:
                                    <ul>
                                        <li>"0000": Mifare Classic</li>
                                        <li>"0001": NFC Forum Typ 1</li>
                                        <li>"0002": NFC Forum Typ 2</li>
                                        <li>"0003": NFC Forum Typ 3</li>
                                        <li>"0004": NFC Forum Typ 4</li>
                                        <li>"0005": NFC Forum Typ 5</li>
                                    </ul>
                                </li>
                            </ul>
                            <strong>Schreiben der Register 4012 und 4013 startet das Vortäuschen das Tags. Danach werden die Holding
                            Register 4000 bis 4013 geleert!</strong> Das Datenformat der Holding Register 4000 bis 4013 ist identisch
                            zum Format der Input Register 4000 bis 4013 (die das zuletzt gesehen NFC-Tag enthalten). Ein physisch
                            existierendes Tag kann also (wieder) vorgetäuscht werden, indem es einmal an die Wallbox gehalten wird
                            und die dabei erzeugten Werte in den Input Registern 4000 bis 4013 später in die Holding Register 4000
                            bis 4013 geschrieben werden.
                        </td>
                    </tr>
                    <tr>
                        <td>4010 bis 4011</td>
                        <td>Verwendungszweck des vorgetäuschten NFC-Tags</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>Siehe Beschreibung der Holding Register 4000 bis 4009.</td>
                    </tr>
                    <tr>
                        <td>4012 bis 4013</td>
                        <td>Typ des vorgetäuschten NFC-Tags</td>
                        <td>uint8 (4x)</td>
                        <td>nfc</td>
                        <td>Siehe Beschreibung der Holding Register 4000 bis 4009.</td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Discrete Inputs (Funktionscodes: Lesen: 2)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>0</td>
                        <td>Feature "evse" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Ein Ladecontroller steht zur Verfügung. Dieses Feature sollte bei allen Wallboxen, deren Hardware
                            funktionsfähig ist, vorhanden sein.</td>
                    </tr>
                    <tr>
                        <td>1</td>
                        <td>Feature "meter" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Ein Stromzähler und Hardware zum Auslesen desselben über RS485 ist verfügbar. Dieses Feature wird erst
                            gesetzt, wenn ein Stromzähler mindestens einmal erfolgreich über Modbus ausgelesen wurde.</td>
                    </tr>
                    <tr>
                        <td>2</td>
                        <td>Feature "phases" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Der verbaute Stromzähler kann Energie und weitere Messwerte einzelner Phasen messen.</td>
                    </tr>
                    <tr>
                        <td>3</td>
                        <td>Feature "all_values" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Der verbaute Stromzähler kann weitere Messwerte auslesen.</td>
                    </tr>
                    <tr>
                        <td>4</td>
                        <td>Feature "phase_switch" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Hardware und Konfiguration erlauben eine Phasenumschaltung.</td>
                    </tr>
                    <tr>
                        <td>5</td>
                        <td>Feature "nfc" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Ein NFC-Bricklet ist verbaut und aktiv.</td>
                    </tr>
                    <tr>
                        <td>6</td>
                        <td>Feature "evse_sd_input" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Der Ladecontroller verfügt über einen Abschalteingang.</td>
                    </tr>
                    <tr>
                        <td>7</td>
                        <td>Feature "evse_gp_input" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Der Ladecontroller verfügt über einen konfigurierbaren Eingang.</td>
                    </tr>
                    <tr>
                        <td>8</td>
                        <td>Feature "evse_gp_output" verfügbar</td>
                        <td>bool</td>
                        <td>---</td>
                        <td>Der Ladecontroller verfügt über einen konfigurierbaren Ausgang.</td>
                    </tr>
                    <tr>
                        <td>1100</td>
                        <td>Zustand des Abschalteingangs</td>
                        <td>bool</td>
                        <td>evse_sd_input</td>
                        <td>0 - geschlossen, 1 - geöffnet</td>
                    </tr>
                    <tr>
                        <td>1101</td>
                        <td>Zustand des konfigurierbaren Eingangs</td>
                        <td>bool</td>
                        <td>evse_gp_input</td>
                        <td>0 - geschlossen, 1 - geöffnet</td>
                    </tr>
                    <tr>
                        <td>1102</td>
                        <td>Zustand des Fronttasters</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>0 - nicht gedrückt, 1 - gedrückt</td>
                    </tr>
                    <tr>
                        <td>2100</td>
                        <td>Phase L1 angeschlossen</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2101</td>
                        <td>Phase L2 angeschlossen</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2102</td>
                        <td>Phase L3 angeschlossen</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2103</td>
                        <td>Phase L1 aktiv</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2104</td>
                        <td>Phase L2 aktiv</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                    <tr>
                        <td>2105</td>
                        <td>Phase L3 aktiv</td>
                        <td>bool</td>
                        <td>phases</td>
                        <td></td>
                    </tr>
                </tbody>
                <thead>
                    <tr>
                        <th colSpan={5}>Coils (Funktionscodes: Lesen: 1, Einzelschreiben: 5, Mehrfachschreiben: 15)</th>
                    </tr>
                </thead>
                <tbody>
                    <tr>
                        <td>1000</td>
                        <td>Ladefreigabe</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>false bzw. 0 zum Blockieren des Ladevorgangs. true bzw. 1 zum Freigeben. Identisch zu Holding
                            Register 1000.</td>
                    </tr>
                    <tr>
                        <td>1001</td>
                        <td>Manuelle Ladefreigabe</td>
                        <td>bool</td>
                        <td>evse</td>
                        <td>false bzw. 0 zum Blockieren des Ladevorgangs. true bzw. 1 zum Freigeben. Setzt die Ladefreigabe, die
                            auch (je nach Konfiguration) vom Taster, den Start/Stop-Buttons auf der Webinterface-Statusseite
                            und der evse/[start/stop]_charging-API verwendet wird.</td>
                    </tr>
                    <tr>
                        <td>1100</td>
                        <td>Setzt den Zustand des konfigurierbaren Ausgangs</td>
                        <td>bool</td>
                        <td>evse_gp_output</td>
                        <td>0 - Verbunden mit Masse, 1 - Hochohmig</td>
                    </tr>
                </tbody>
            </>
        },
        "script": {
            "save_failed": "Speichern der Modbus/TCP-Einstellungen fehlgeschlagen."
        }
    }
}
