FROM continuumio/miniconda3

WORKDIR /app

COPY environment.yml .
RUN conda env create -f environment.yml
RUN echo "conda activate scan3d" >> ~/.bashrc
COPY requirements.txt .
RUN bash -c "source activate scan3d && pip install -r requirements.txt"
SHELL ["bash", "--login", "-c"]
RUN export PATH=/opt/conda/envs/scan3d/bin:$PATH

COPY data data
COPY src src

CMD ["conda", "run", "--no-capture-output", "-n", "scan3d", "python", "-m", "src.scanner3d"]
