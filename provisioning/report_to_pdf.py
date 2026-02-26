#!/usr/bin/env -S uv run
# /// script
# requires-python = ">=3.12"
# dependencies = []
# ///

import os
import sys
import argparse
import json
import tempfile
import shutil
from datetime import datetime


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    parser.add_argument('output_file')

    args = parser.parse_args()

    with open(args.input_file, 'r') as f:
        report = json.load(f)

    tests = report['electrical_tests']

    latex = rf'''
\documentclass[a4paper,11pt]{{article}}

\usepackage[margin=15mm]{{geometry}}
\usepackage[utf8]{{inputenc}}
\usepackage[ngerman]{{babel}}
\usepackage[T1]{{fontenc}}
\usepackage{{lmodern}}
\usepackage{{booktabs}}
\usepackage{{multicol}}
\usepackage{{tabularx}}
\usepackage{{tikzpagenodes}}
\usepackage{{graphicx}}

\renewcommand{{\familydefault}}{{\sfdefault}}

\begin{{document}}
\pagestyle{{empty}}

\begin{{tikzpicture}}[remember picture,overlay,shift={{(current page.north west)}}]
\node[anchor=north west,xshift=13mm,yshift=-14mm]{{\huge\textbf{{{[x[2:] for x in report["qr_code"].split(";") if x.startswith("T:")][0].split("-")[0]} Charger}}}};
\end{{tikzpicture}}

\begin{{tikzpicture}}[remember picture,overlay,shift={{(current page.north west)}}]
\node[anchor=north west,xshift=13mm,yshift=-23mm]{{\huge\textbf{{Prüfprotokoll}}}};
\end{{tikzpicture}}

\begin{{tikzpicture}}[remember picture,overlay,shift={{(current page.north east)}}]
\node[anchor=north east,xshift=-13.5mm,yshift=-14mm]{{\includegraphics[width=60mm]{{logo}}}};
\end{{tikzpicture}}

\vspace{{8mm}}
\begin{{multicols}}{{2}}

\section*{{Prüfling}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Modell           & {[x[2:] for x in report["qr_code"].split(";") if x.startswith("T:")][0]} \\
Firmware-Version & {report["firmware_version"]} \\
Seriennummer     & {report["serial"]} \\
Prüfdatum        & {datetime.fromisoformat(report["start"]).astimezone().strftime("%d.%m.%Y %H:%M:%S")} \\
\bottomrule
\end{{tabularx}}

\section*{{Messgerät}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Modell           & {tests["device_info"]["product_name"]} {tests["device_info"]["model_name"]} \\
Firmware-Version & {tests["device_info"]["firmware_version"]} \\
Seriennummer     & {tests["device_info"]["serial_number"]} \\
Kalibrierdatum   & {tests["device_info"]["calibration_date"]} \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Messergebnis: Spannungen}}

\begin{{tabularx}}{{\textwidth}}{{p{{20mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}X}}
\toprule
                        & L1                                                          & L2                                                          & L3                                                          & \\
\midrule
U\textsubscript{{L-N}}  & {tests["voltage_L1"]["results"]["Uln"]}                     & {tests["voltage_L2"]["results"]["Uln"]}                     & {tests["voltage_L3"]["results"]["Uln"]}                     & bestanden \\
U\textsubscript{{L-PE}} & {tests["voltage_L1"]["results"]["Ulpe"]}                    & {tests["voltage_L2"]["results"]["Ulpe"]}                    & {tests["voltage_L3"]["results"]["Ulpe"]}                    & bestanden \\
U\textsubscript{{N-PE}} & {tests["voltage_L1"]["results"]["Unpe"].replace(".", ",")}  & {tests["voltage_L2"]["results"]["Unpe"].replace(".", ",")}  & {tests["voltage_L3"]["results"]["Unpe"].replace(".", ",")}  & bestanden \\
\midrule
Frequenz                & {tests["voltage_L1"]["results"]["FreqV"].replace(".", ",")} & {tests["voltage_L2"]["results"]["FreqV"].replace(".", ",")} & {tests["voltage_L3"]["results"]["FreqV"].replace(".", ",")} & \\
\bottomrule
\end{{tabularx}}

\vspace{{4mm}}
\begin{{multicols}}{{2}}

\subsection*{{Grenzwerte}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Unteres Limit (U\textsubscript{{L-N}})  & {tests["voltage_L1"]["limits"]["MI3155_Uln_LoLim_Percent"].replace("Percent", r"\%").replace("-", "$-$")} \\
Oberes Limit (U\textsubscript{{L-N}})   & +{tests["voltage_L1"]["limits"]["MI3155_Uln_HiLim_Percent"].replace("Percent", r"\%")} \\
Unteres Limit (U\textsubscript{{L-PE}}) & {tests["voltage_L1"]["limits"]["MI3155_Ulpe_LoLim"]} \\
Oberes Limit (U\textsubscript{{L-PE}})  & {tests["voltage_L1"]["limits"]["MI3155_Ulpe_HiLim"]} \\
Unteres Limit (U\textsubscript{{N-PE}}) & {tests["voltage_L1"]["limits"]["MI3155_Unpe_LoLim"]} \\
Oberes Limit (U\textsubscript{{N-PE}})  & {tests["voltage_L1"]["limits"]["MI3155_Unpe_HiLim"]} \\
\bottomrule
\end{{tabularx}}

\subsection*{{Parameter}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Spannungsversorgung  & {tests["voltage_L1"]["parameters"]["Voltage_system"].replace("phase", "phasig")} \\
Grenzwerttyp         & {tests["voltage_L1"]["parameters"]["Voltage_limit_type"].replace("%25", r"\%")} \\
Erdungssystem        & {tests["voltage_L1"]["parameters"]["Earthing_System"]} \\
Nennspannung         & {tests["voltage_L1"]["parameters"]["Nominal_voltage"]} \\
Dauer                & {tests["voltage_L1"]["parameters"]["Test_time_Voltage_INST"]} \\
%Test                & {tests["voltage_L1"]["parameters"]["Voltage_Test"]} \\
                     & \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Messergebnis: Z auto}}

\begin{{tabularx}}{{\textwidth}}{{p{{20mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}X}}
\toprule
                         & L1                                                                                        & L2                                                                                        & L3                                                                                        & \\
\midrule
U\textsubscript{{L-N}}   & {tests["zauto_L1"]["results"]["Uln"]}                                                     & {tests["zauto_L2"]["results"]["Uln"]}                                                     & {tests["zauto_L3"]["results"]["Uln"]}                                                     & \\
Z\textsubscript{{L-PE}}  & {tests["zauto_L1"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["zauto_L2"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["zauto_L3"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & \\
Z\textsubscript{{L-N}}   & {tests["zauto_L1"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & {tests["zauto_L2"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & {tests["zauto_L3"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & \\
\midrule
Ik\textsubscript{{L-N}}  & {tests["zauto_L1"]["results"]["IPSC_LN_Auto"]}                                            & {tests["zauto_L2"]["results"]["IPSC_LN_Auto"]}                                            & {tests["zauto_L3"]["results"]["IPSC_LN_Auto"]}                                            & bestanden \\
Ik\textsubscript{{L-PE}} & {tests["zauto_L1"]["results"]["IPSC_LPE_Auto"]}                                           & {tests["zauto_L2"]["results"]["IPSC_LPE_Auto"]}                                           & {tests["zauto_L3"]["results"]["IPSC_LPE_Auto"]}                                           & bestanden \\
Uc                       & {tests["zauto_L1"]["results"]["Uc"].replace(".", ",")}                                    & {tests["zauto_L2"]["results"]["Uc"].replace(".", ",")}                                    & {tests["zauto_L3"]["results"]["Uc"].replace(".", ",")}                                    & bestanden \\
\bottomrule
\end{{tabularx}}

\vspace{{4mm}}
\begin{{multicols}}{{2}}

\subsection*{{Grenzwerte}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Limit ($\Delta$U)                                      & {tests["zauto_L1"]["limits"]["Limit_VVoltage_Drop"].replace(".", ",").replace("Percent", r"\%")} \\
Ia (Ik\textsubscript{{L-N}}, Ik\textsubscript{{L-PE}}) & {tests["zauto_L1"]["limits"]["Fuse_Ia"].replace("Percent", r"\%")} \\
Limit (Uc)                                             & {tests["zauto_L1"]["limits"]["UC_lim"]} \\
                                                       & \\
                                                       & \\
                                                       & \\
                                                       & \\
                                                       & \\
\bottomrule
\end{{tabularx}}

\subsection*{{Parameter}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
%Sicherung / RCD & {tests["zauto_L1"]["parameters"]["Protection_fuse_rcd"].replace("Fuse(LN,LPE)_RCD", "Sicherung (L-N, L-PE) und RCD")} \\
Sicherungstyp    & {tests["zauto_L1"]["parameters"]["FuseType"]} \\
Sicherung In     & {tests["zauto_L1"]["parameters"]["Fuse_Ir"]} \\
Ik-Faktor        & {tests["zauto_L1"]["parameters"]["Isc_factor"]} \\
RCD Typ          & {tests["zauto_L1"]["parameters"]["TYPE_RCD"]} \\
I $\Delta$N      & {tests["zauto_L1"]["parameters"]["I_dN"]} \\
Empfindlichkeit  & {tests["zauto_L1"]["parameters"]["RCD_Selectivity"]} \\
%Phase           & {tests["zauto_L1"]["parameters"]["Phase_Z_Auto"]} \\
Prüfstrom I      & {tests["zauto_L1"]["parameters"]["I_test"].replace("Low", "niedrig")} \\
Abschaltzeit t   & {tests["zauto_L1"]["parameters"]["Fuse_t"].replace(".", ",")} \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Messergebnis: RCD I}}

\begin{{tabularx}}{{\textwidth}}{{p{{20mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}X}}
\toprule
                 & Positiv                                                        & Negativ                                                        & \\
\midrule
I $\Delta$N      & {tests["rcdi_positive"]["results"]["Id"].replace(".", ",")}    & {tests["rcdi_negative"]["results"]["Id"].replace(".", ",")}    & bestanden \\
\midrule
t (I $\Delta$N)  & {tests["rcdi_positive"]["results"]["t_Id"].replace(".", ",")}  & {tests["rcdi_negative"]["results"]["t_Id"].replace(".", ",")}  & \\
Uc (I $\Delta$N) & {tests["rcdi_positive"]["results"]["Uc_Id"].replace(".", ",")} & {tests["rcdi_negative"]["results"]["Uc_Id"].replace(".", ",")} & \\
Uc               & {tests["rcdi_positive"]["results"]["Uc"].replace(".", ",")}    & {tests["rcdi_negative"]["results"]["Uc"].replace(".", ",")}    & \\
\bottomrule
\end{{tabularx}}

\vspace{{4mm}}
\begin{{multicols}}{{2}}

\subsection*{{Grenzwerte}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Limit (Uc) & {tests["rcdi_positive"]["limits"]["UC_lim"]} \\
           & \\
           & \\
           & \\
           & \\
           & \\
\bottomrule
\end{{tabularx}}

\subsection*{{Parameter}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
RCD Bauart                   & {tests["rcdi_positive"]["parameters"]["RCD_use"].replace("other", "EVSE / MI")} \\
Typ                          & {tests["rcdi_positive"]["parameters"]["Other_RCD_Type"]} \\
I $\Delta$N / I $\Delta$N DC & {tests["rcdi_positive"]["parameters"]["IdN_IdnDC"].replace("d.c.", "DC")} \\
Prüfung                      & {tests["rcdi_positive"]["parameters"]["Other_RCD_Test"].replace("d.c.", "DC")} \\
%Prüfung zwischen            & {tests["rcdi_positive"]["parameters"]["ZLoop_Test"]} \\
RCD Prüfnorm                 & {tests["rcdi_positive"]["parameters"]["RCD_Standard"]} \\
Erdungssystem                & {tests["rcdi_positive"]["parameters"]["Earthing_System"]} \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Messergebnis: R iso}}

\begin{{tabularx}}{{\textwidth}}{{p{{20mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}X}}
\toprule
     & L1                                                                                  & L2                                                                                  & L3                                                                                  & N                                                                                  & \\
\midrule
Riso & {tests["riso_L1"]["results"]["Riso"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["riso_L2"]["results"]["Riso"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["riso_L3"]["results"]["Riso"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["riso_N"]["results"]["Riso"].replace(".", ",").replace("Ohm", r"$\Omega$")} & bestanden \\
\midrule
Um   & {tests["riso_L1"]["results"]["Um"]}                                                 & {tests["riso_L2"]["results"]["Um"]}                                                 & {tests["riso_L3"]["results"]["Um"]}                                                 & {tests["riso_N"]["results"]["Um"]}                                                 & \\
\bottomrule
\end{{tabularx}}

\vspace{{4mm}}
\begin{{multicols}}{{2}}

\subsection*{{Grenzwerte}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Limit (Riso) & {tests["riso_L1"]["limits"]["Limit_Riso"].replace("Ohm", r"$\Omega$")} \\
\bottomrule
\end{{tabularx}}

\subsection*{{Parameter}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Uiso & {tests["riso_L1"]["parameters"]["Uiso_INST_EE"]} \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Messergebnis: R low}}

\begin{{tabularx}}{{\textwidth}}{{p{{20mm}} >{{\raggedleft\arraybackslash}}p{{22mm}} >{{\raggedleft\arraybackslash}}X}}
\toprule
R          & {tests["rlow"]["results"]["R_cont"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & bestanden \\
\midrule
R$+$       & {tests["rlow"]["results"]["R_Plus"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & \\
R$-$       & {tests["rlow"]["results"]["R_Minus"].replace(".", ",").replace("Ohm", r"$\Omega$")} & \\
Kalibriert & {tests["rlow"]["results"]["Cal_R"].replace("Yes", "Ja").replace("No", "Nein")}      & \\
\bottomrule
\end{{tabularx}}

%\vspace{{4mm}}
\vspace{{0.9mm}}
\begin{{multicols}}{{2}}

\subsection*{{Grenzwerte}}

\begin{{tabularx}}{{0.475\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Limit (R) & {tests["rlow"]["limits"]["Limit_Rlow"].replace("Ohm", r"$\Omega$")} \\
          & \\
          & \\
\bottomrule
\end{{tabularx}}

\subsection*{{Parameter}}

\begin{{tabularx}}{{0.49\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
Anschluss       & {tests["rlow"]["parameters"]["Type_cont_out"].replace("LPE", "L-PE")} \\
Masseverbindung & {tests["rlow"]["parameters"]["Bonding_type"]} \\
Strom           & {tests["rlow"]["parameters"]["RLow_Current_type"].replace("standard", "Standard")} \\
\bottomrule
\end{{tabularx}}

\end{{multicols}}

\vspace{{2mm}}
\section*{{Funktionsprüfung EVSE}}

\begin{{tabularx}}{{\textwidth}}{{l >{{\raggedleft\arraybackslash}}X}}
\toprule
IEC 61851 Status A -- Standby                          & bestanden \\
IEC 61851 Status B -- EV erkannt, spannungslos         & bestanden \\
IEC 61851 Status C -- Spannung liegt an, EV lädt       & bestanden \\
IEC 61851 Status E -- Fehler, Ladevorgang unterbrochen & bestanden \\
PP (I\textsubscript{{max}})                            & bestanden \\
\bottomrule
\end{{tabularx}}

\end{{document}}
'''

    output_file = os.path.abspath(args.output_file)

    with tempfile.TemporaryDirectory() as d:
        print(d)

        shutil.copy('logo.pdf', d)

        os.chdir(d)

        with open('report.tex', 'w') as f:
            f.write(latex)

        if os.system('pdflatex -halt-on-error -interaction nonstopmode report.tex') != 0:
            return 1

        if os.system('pdflatex -halt-on-error -interaction nonstopmode report.tex') != 0:
            return 2

        if os.system(f'ps2pdf14 -dPDFSETTINGS=/prepress -dSubsetFonts=false -dCompressFonts=true report.pdf {output_file}') != 0:
            return 3

    return 0


if __name__ == '__main__':
    sys.exit(main())
