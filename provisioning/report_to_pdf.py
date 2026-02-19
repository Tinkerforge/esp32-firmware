#!/usr/bin/env -S uv run
# /// script
# requires-python = ">=3.12"
# dependencies = []
# ///

import os
import argparse
import json
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_file')
    parser.add_argument('output_file')

    args = parser.parse_args()

    with open(args.input_file, 'r') as f:
        report = json.load(f)

    tests = report['electrical_tests']

    latex = rf'''
\documentclass{{scrarticle}}

\usepackage[paper=a4paper,left=25mm,right=25mm,top=25mm,bottom=25mm]{{geometry}}
\usepackage[utf8]{{inputenc}}
\usepackage[ngerman]{{babel}}
\usepackage[T1]{{fontenc}}
\usepackage{{lmodern}}
\usepackage{{booktabs}}

\renewcommand{{\familydefault}}{{\sfdefault}}

\begin{{document}}
\pagestyle{{empty}}

\section*{{Wallbox}}

\begin{{tabular}}{{ll}}
\toprule
Seriennummer & {report["serial"]} \\
\bottomrule
\end{{tabular}}

\section*{{Messgeräte}}

\begin{{tabular}}{{ll}}
\toprule
Modell           & {tests["device_info"]["product_name"]} {tests["device_info"]["model_name"]} \\
Firmware-Version & {tests["device_info"]["firmware_version"]} \\
Seriennummer     & {tests["device_info"]["serial_number"]} \\
Kalibrierdatum   & {tests["device_info"]["calibration_date"]} \\
\bottomrule
\end{{tabular}}

\section*{{Testergebnisse}}

\subsection*{{Test 1: Funktionsprüfung EVSE [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{ll}}
\toprule
CP Schalterstellungen & bestanden \\
Stellung A - EVSE im Ruhezustand & bestanden \\
Stellung B - EV erkannt, Verriegelung aktiv, Spannungslos & bestanden \\
Stellung C - EV läd, Spannung liegt an & bestanden \\
Stellung E - Fehler, Ladevorgang unterbrochen & bestanden \\
PP (I\textsubscript{{max}}) & bestanden \\
\bottomrule
\end{{tabular}}

\subsection*{{Test 2: Spannungen [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{lrrrl}}
\toprule
                        & L1                                                           & L2                                                           & L3                                                           & \\
\midrule
U\textsubscript{{L-N}}  & {tests["voltage_L1"]["results"]["Uln"]}                     & {tests["voltage_L2"]["results"]["Uln"]}                     & {tests["voltage_L3"]["results"]["Uln"]}                     & bestanden \\
U\textsubscript{{L-PE}} & {tests["voltage_L1"]["results"]["Ulpe"]}                    & {tests["voltage_L2"]["results"]["Ulpe"]}                    & {tests["voltage_L3"]["results"]["Ulpe"]}                    & bestanden \\
U\textsubscript{{N-PE}} & {tests["voltage_L1"]["results"]["Unpe"].replace(".", ",")}  & {tests["voltage_L2"]["results"]["Unpe"].replace(".", ",")}  & {tests["voltage_L3"]["results"]["Unpe"].replace(".", ",")}  & bestanden \\
\midrule
Frequenz                & {tests["voltage_L1"]["results"]["FreqV"].replace(".", ",")} & {tests["voltage_L2"]["results"]["FreqV"].replace(".", ",")} & {tests["voltage_L3"]["results"]["FreqV"].replace(".", ",")} & \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Grenzwerte}}

\begin{{tabular}}{{lr}}
\toprule
Unteres Limit (U\textsubscript{{L-N}})  & {tests["voltage_L1"]["limits"]["MI3155_Uln_LoLim_Percent"].replace("Percent", r"\%")} \\
Oberes Limit (U\textsubscript{{L-N}})   & {tests["voltage_L1"]["limits"]["MI3155_Uln_HiLim_Percent"].replace("Percent", r"\%")} \\
Unteres Limit (U\textsubscript{{L-PE}}) & {tests["voltage_L1"]["limits"]["MI3155_Ulpe_LoLim"]} \\
Oberes Limit (U\textsubscript{{L-PE}})  & {tests["voltage_L1"]["limits"]["MI3155_Ulpe_HiLim"]} \\
Unteres Limit (U\textsubscript{{N-PE}}) & {tests["voltage_L1"]["limits"]["MI3155_Unpe_LoLim"]} \\
Oberes Limit (U\textsubscript{{N-PE}})  & {tests["voltage_L1"]["limits"]["MI3155_Unpe_HiLim"]} \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Parameter}}

\begin{{tabular}}{{lr}}
\toprule
Spannungsversorgung  & {tests["voltage_L1"]["parameters"]["Voltage_system"].replace("phase", "phasig")} \\
Grenzwerttyp         & {tests["voltage_L1"]["parameters"]["Voltage_limit_type"].replace("%25", r"\%")} \\
Erdungssystem        & {tests["voltage_L1"]["parameters"]["Earthing_System"]} \\
Nennspannung         & {tests["voltage_L1"]["parameters"]["Nominal_voltage"]} \\
Dauer                & {tests["voltage_L1"]["parameters"]["Test_time_Voltage_INST"]} \\
Test                 & {tests["voltage_L1"]["parameters"]["Voltage_Test"]} \\
\bottomrule
\end{{tabular}}

\subsection*{{Test 3: Z auto [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{lrrrl}}
\toprule
                         & L1                                                                                         & L2                                                                                         & L3                                                                                         & \\
\midrule
U\textsubscript{{L-N}}   & {tests["zauto_L1"]["results"]["Uln"]}                                                     & {tests["zauto_L2"]["results"]["Uln"]}                                                     & {tests["zauto_L3"]["results"]["Uln"]}                                                     & \\
Z\textsubscript{{L-PE}}  & {tests["zauto_L1"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["zauto_L2"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & {tests["zauto_L3"]["results"]["ZPLE_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")} & \\
Z\textsubscript{{L-N}}   & {tests["zauto_L1"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & {tests["zauto_L2"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & {tests["zauto_L3"]["results"]["ZLN_Auto"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & \\
\midrule
Ik\textsubscript{{L-N}}  & {tests["zauto_L1"]["results"]["IPSC_LN_Auto"]}                                            & {tests["zauto_L2"]["results"]["IPSC_LN_Auto"]}                                            & {tests["zauto_L3"]["results"]["IPSC_LN_Auto"]}                                            & bestanden \\
Ik\textsubscript{{L-PE}} & {tests["zauto_L1"]["results"]["IPSC_LPE_Auto"]}                                           & {tests["zauto_L2"]["results"]["IPSC_LPE_Auto"]}                                           & {tests["zauto_L3"]["results"]["IPSC_LPE_Auto"]}                                           & bestanden \\
Uc                       & {tests["zauto_L1"]["results"]["Uc"].replace(".", ",")}                                    & {tests["zauto_L2"]["results"]["Uc"].replace(".", ",")}                                    & {tests["zauto_L3"]["results"]["Uc"].replace(".", ",")}                                    & bestanden \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Grenzwerte}}

\begin{{tabular}}{{lr}}
\toprule
Limit ($\Delta$U)                                      & {tests["zauto_L1"]["limits"]["Limit_VVoltage_Drop"].replace(".", ",").replace("Percent", r"\%")} \\
Ia (Ik\textsubscript{{L-N}}, Ik\textsubscript{{L-PE}}) & {tests["zauto_L1"]["limits"]["Fuse_Ia"].replace("Percent", r"\%")} \\
Limit (Uc)                                             & {tests["zauto_L1"]["limits"]["UC_lim"]} \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Parameter}}

\begin{{tabular}}{{lr}}
\toprule
%Sicherung / RCD & {tests["zauto_L1"]["parameters"]["Protection_fuse_rcd"].replace("Fuse(LN,LPE)_RCD", "Sicherung (L-N, L-PE) und RCD")} \\
Sicherungstyp    & {tests["zauto_L1"]["parameters"]["FuseType"]} \\
Sicherung In     & {tests["zauto_L1"]["parameters"]["Fuse_Ir"]} \\
Ik-Faktor        & {tests["zauto_L1"]["parameters"]["Isc_factor"]} \\
RCD Typ          & {tests["zauto_L1"]["parameters"]["TYPE_RCD"]} \\
I $\Delta$N      & {tests["zauto_L1"]["parameters"]["I_dN"]} \\
Empfindlichkeit  & {tests["zauto_L1"]["parameters"]["RCD_Selectivity"]} \\
Phase            & {tests["zauto_L1"]["parameters"]["Phase_Z_Auto"]} \\
Prüfstrom I      & {tests["zauto_L1"]["parameters"]["I_test"].replace("Low", "niedrig")} \\
Abschaltzeit t   & {tests["zauto_L1"]["parameters"]["Fuse_t"].replace(".", ",")} \\
\bottomrule
\end{{tabular}}

\subsection*{{Test 4: RCD I [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{lrrl}}
\toprule
                 & Positiv                                                         & Negativ                                                         & \\
\midrule
I $\Delta$N      & {tests["rcdi_positive"]["results"]["Id"].replace(".", ",")}    & {tests["rcdi_negative"]["results"]["Id"].replace(".", ",")}    & bestanden \\
\midrule
t (I $\Delta$N)  & {tests["rcdi_positive"]["results"]["t_Id"].replace(".", ",")}  & {tests["rcdi_negative"]["results"]["t_Id"].replace(".", ",")}  & \\
Uc (I $\Delta$N) & {tests["rcdi_positive"]["results"]["Uc_Id"].replace(".", ",")} & {tests["rcdi_negative"]["results"]["Uc_Id"].replace(".", ",")} & \\
Uc               & {tests["rcdi_positive"]["results"]["Uc"].replace(".", ",")}    & {tests["rcdi_negative"]["results"]["Uc"].replace(".", ",")}    & \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Grenzwerte}}

\begin{{tabular}}{{lr}}
\toprule
Limit (Uc) & {tests["rcdi_positive"]["limits"]["UC_lim"]} \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Parameter}}

\begin{{tabular}}{{lr}}
\toprule
RCD Bauart                   & {tests["rcdi_positive"]["parameters"]["RCD_use"]} \\
Typ                          & {tests["rcdi_positive"]["parameters"]["Other_RCD_Type"]} \\
I $\Delta$N / I $\Delta$N DC & {tests["rcdi_positive"]["parameters"]["IdN_IdnDC"]} \\
Prüfung                      & {tests["rcdi_positive"]["parameters"]["Other_RCD_Test"]} \\
Prüfung zwischen             & {tests["rcdi_positive"]["parameters"]["ZLoop_Test"]} \\
RCD Prüfnorm                 & {tests["rcdi_positive"]["parameters"]["RCD_Standard"]} \\
Erdungssystem                & {tests["rcdi_positive"]["parameters"]["Earthing_System"]} \\
\bottomrule
\end{{tabular}}

\subsection*{{Test 5: R iso [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{lrrrrl}}
\toprule
     & L1                                                                 & L2                                                                 & L3                                                                 & N                                                                 & \\
\midrule
Riso & {tests["riso_L1"]["results"]["Riso"].replace("Ohm", r"$\Omega$")} & {tests["riso_L2"]["results"]["Riso"].replace("Ohm", r"$\Omega$")} & {tests["riso_L3"]["results"]["Riso"].replace("Ohm", r"$\Omega$")} & {tests["riso_N"]["results"]["Riso"].replace("Ohm", r"$\Omega$")} & bestanden \\
\midrule
Um   & {tests["riso_L1"]["results"]["Um"]}                               & {tests["riso_L2"]["results"]["Um"]}                               & {tests["riso_L3"]["results"]["Um"]}                               & {tests["riso_N"]["results"]["Um"]}                               & \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Grenzwerte}}

\begin{{tabular}}{{lr}}
\toprule
Limit (Riso)  & {tests["riso_L1"]["limits"]["Limit_Riso"].replace("Ohm", r"$\Omega$")} \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Parameter}}

\begin{{tabular}}{{lr}}
\toprule
Uiso & {tests["riso_L1"]["parameters"]["Uiso_INST_EE"]} \\
\bottomrule
\end{{tabular}}

\subsection*{{Test 6: R low [bestanden]}}

\subsubsection*{{Ergebnisse}}

\begin{{tabular}}{{lrl}}
\toprule
R          & {tests["rlow"]["results"]["R_cont"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & bestanden \\
\midrule
R$+$       & {tests["rlow"]["results"]["R_Plus"].replace(".", ",").replace("Ohm", r"$\Omega$")}  & \\
R$-$       & {tests["rlow"]["results"]["R_Minus"].replace(".", ",").replace("Ohm", r"$\Omega$")} & \\
Kalibriert & {tests["rlow"]["results"]["Cal_R"].replace("Yes", "Ja").replace("No", "Nein")}      & \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Grenzwerte}}

\begin{{tabular}}{{lr}}
\toprule
Limit (R) & {tests["rlow"]["limits"]["Limit_Rlow"].replace("Ohm", r"$\Omega$")} \\
\bottomrule
\end{{tabular}}

\subsubsection*{{Parameter}}

\begin{{tabular}}{{lr}}
\toprule
Anschluss       & {tests["rlow"]["parameters"]["Type_cont_out"].replace("LPE", "L-PE")} \\
Masseverbindung & {tests["rlow"]["parameters"]["Bonding_type"]} \\
Strom           & {tests["rlow"]["parameters"]["RLow_Current_type"].replace("standard", "Standard")} \\
\bottomrule
\end{{tabular}}

\end{{document}}
'''

    output_file = os.path.abspath(args.output_file)

    with tempfile.TemporaryDirectory() as d:
        print(d)
        os.chdir(d)

        with open('report.tex', 'w') as f:
            f.write(latex)

        os.system('pdflatex report.tex')
        os.system('pdflatex report.tex')
        os.system(f'ps2pdf14 -dPDFSETTINGS=/prepress -dSubsetFonts=false -dCompressFonts=true report.pdf {output_file}')


if __name__ == '__main__':
    main()
