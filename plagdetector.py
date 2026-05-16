import streamlit as st
import pandas as pd
import subprocess
import joblib
import os

# --------------------------
# Config paths
# --------------------------
MODEL_PATH =  r"C:\Users\hp\Downloads\ds project final\model.pkl"
EXE_PATH = r"C:\Users\hp\Downloads\ds project final\computeNew.exe"
INPUT_PATH = r"C:\Users\hp\Downloads\ds project final\input.txt"
CSV_PATH = r"C:\Users\hp\Downloads\ds project final\new_features.csv"

# Load ML model
model = joblib.load(MODEL_PATH)

# --------------------------
# Streamlit UI
# --------------------------
st.set_page_config(page_title="Plagiarism Detector", page_icon="🔍", layout="wide")
st.title("🔍 Plagiarism Detection System (C++ + ML)")

uploaded = st.file_uploader("Upload a text file", type=["txt", "csv", "md", "docx"])

if uploaded:
    # Extract text
    text = uploaded.read().decode("utf-8", errors="ignore")
    st.subheader("📄 Uploaded Text")
    st.text_area("Text Preview", text, height=150)

    # Save text for C++ program
    with open(INPUT_PATH, "w", encoding="utf-8") as f:
        f.write(text)

    # Run C++ EXE
    st.info("⚙️ Running feature extraction...")
    if not os.path.exists(EXE_PATH):
        st.error("❌ compute_new.exe not found!")
    else:
        subprocess.run([EXE_PATH])

        # Load features
        if os.path.exists(CSV_PATH):
            df = pd.read_csv(CSV_PATH)

            # Show table of features
            st.subheader("📊 Extracted Features")
            st.dataframe(df)

            # Show features as cards
            features = df.iloc[0]  # assuming single-row input
            st.subheader("💡 Feature Summary")
            col1, col2, col3, col4 = st.columns(4)
            col1.metric("Prefix Match", features["prefix"])
            col2.metric("Substr(10)", features["sub10"])
            col3.metric("Longest Common Substring", features["lcs"])
            col4.metric("Word Overlap", features["inverted"])

            # ML prediction
            X_new = df[["prefix", "sub10", "lcs", "inverted"]]
            prediction = model.predict(X_new)[0]

            st.subheader("🔎 Plagiarism Result")
            if prediction == 1:
                st.error("❗ Plagiarism Detected!")
            else:
                st.success("✅ Not Plagiarized")

            # Optional: bar chart of feature scores
            st.subheader("📊 Feature Comparison")
            feature_chart = pd.DataFrame({
                "Feature": ["Prefix", "Substr10", "LCS", "Inverted"],
                "Score": [features["prefix"], features["sub10"], features["lcs"], features["inverted"]]
            })
            st.bar_chart(feature_chart.set_index("Feature"))

        else:
            st.error("❌ Feature CSV (new_features.csv) not found!")
