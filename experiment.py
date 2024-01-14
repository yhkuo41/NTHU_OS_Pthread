import csv
import math
import subprocess
import time

import matplotlib.pyplot as plt

case = "01"
specs = {"00": 200, "01": 4000}


def run_shell_command(command, log=""):
    try:
        print(command)
        start_time = time.time()
        if log:
            with open(f"{log}.log", "a") as f:
                f.write(f"\n{command}\n")
                subprocess.run(command, check=True, shell=True, stdout=f)
        else:
            subprocess.run(command, check=True, shell=True)
        return time.time() - start_time
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        exit(1)


def single_variable_experiment(keyword, lo, hi, exp_args, step=1, filename="", file_op="w"):
    if not filename:
        filename = keyword
    res = dict()
    test_case_lines = specs[case]
    for i in range(lo, hi, step):
        exp_args[keyword] = i
        cmd = f"sh ./run_pthread {case} {test_case_lines} {0}"
        for v in exp_args.values():
            cmd += f" {v}"
        exec_time = run_shell_command(cmd, filename)
        res[i] = exec_time

    write_csv(res, filename, file_op)


def qsize_exp(lo, hi, exp_args, step=1):
    res = dict()
    filename = "qsize"
    test_case_lines = specs[case]
    for i in range(lo, hi, step):
        exp_args["reader_q_size"] = i
        exp_args["worker_q_size"] = i
        exp_args["writer_q_size"] = i
        cmd = f"sh ./run_pthread {case} {test_case_lines} {0}"
        for v in exp_args.values():
            cmd += f" {v}"
        exec_time = run_shell_command(cmd, filename)
        res[i] = exec_time
    write_csv(res, filename)


def qsize_cc_period_prod_exp(exp_args):
    res = []
    filename = "qsize_cc_period_prod"
    periods = [5, 10, 100, 1000, 10000, 100000, 1000000, 10000000]
    test_case_lines = specs[case]
    for p in periods:
        for i in range(1, 52, 5):
            exp_args["cc_period"] = p
            exp_args["producer_num"] = i
            cmd = f"sh ./run_pthread {case} {test_case_lines} {0}"
            for v in exp_args.values():
                cmd += f" {v}"
            exec_time = run_shell_command(cmd, filename)
            res.append((p, i, exec_time))
    csv = filename + ".csv"
    print("write to ", csv)
    with open(csv, "w") as file:
        for period, prod_num, exec_time in res:
            file.write(f"{period},{prod_num},{exec_time}\n")


def cc_period_exp(exp_args):
    periods = [5, 10, 100, 1000, 10000, 100000, 1000000, 10000000]
    for p in periods:
        single_variable_experiment("cc_period", p, p + 1, exp_args.copy(), 1000000, "cc_period", "a")


def plot(csv_file, x_axis, xscale="linear"):
    exec_times = []
    x_data = []
    with open(csv_file, 'r') as f:
        csv_reader = csv.reader(f)
        for row in csv_reader:
            x_data.append(int(row[0]))
            exec_times.append(float(row[1]))

    plt.plot(x_data, exec_times, marker='o')
    plt.title(f'Execution Time (seconds) vs {x_axis}')
    plt.xlabel(x_axis)
    plt.xscale(xscale)
    plt.ylabel('Execution Time (seconds)')
    plt.grid(True)
    plt.savefig(csv_file.replace("csv", "png"))
    plt.close()


def plot_cc_period_prod_exp():
    exec_times = []
    producers = []
    periods = []
    csv_file = "cc_period_prod.csv"
    with open(csv_file, 'r') as f:
        csv_reader = csv.reader(f)
        for row in csv_reader:
            periods.append(math.log(int(row[0])))
            producers.append(int(row[1]))
            exec_times.append(float(row[2]))
    ax = plt.subplot(projection='3d')
    ax.scatter(producers, periods, exec_times, s=90, c=periods)
    ax.set_xlabel('producer_num')
    ax.set_ylabel('log(cc_period) (ms)')
    ax.set_zlabel('Execution Time (seconds)')
    plt.show()


def write_csv(res, filename, file_op="w"):
    filename = filename + ".csv"
    print("write to ", filename)
    with open(filename, file_op) as file:
        for k, v in res.items():
            file.write(f"{k},{v}\n")


if __name__ == "__main__":
    experiment_args = {
        "reader_q_size": 200,
        "worker_q_size": 200,
        "writer_q_size": 4000,
        "cc_lo_p": 20,
        "cc_hi_p": 80,
        "cc_period": 1000000,
        "producer_num": 4,
    }
    # run_shell_command(f"sh ./build_pthread {case}")

    # single variable
    # single_variable_experiment("reader_q_size", 1, 202, experiment_args.copy(), 10, "reader_q_size")
    # single_variable_experiment("reader_q_size", 301, 4002, experiment_args.copy(), 200, "reader_q_size", "a")
    # single_variable_experiment("worker_q_size", 1, 4002, experiment_args.copy(), 100)
    # single_variable_experiment("writer_q_size", 1, 202, experiment_args.copy(), 20, "writer_q_size")
    # single_variable_experiment("writer_q_size", 301, 8002, experiment_args.copy(), 500, "writer_q_size", "a")
    # single_variable_experiment("cc_lo_p", 1, 80, experiment_args.copy(), 5)
    # single_variable_experiment("cc_hi_p", 21, 98, experiment_args.copy(), 5)
    # single_variable_experiment("producer_num", 1, 52, experiment_args.copy(), 5)
    # cc_period_exp(experiment_args.copy())

    # multivariate
    # qsize_exp(1, 4002, experiment_args.copy(), 200)
    # qsize_cc_period_prod_exp(experiment_args.copy())

    plot_cc_period_prod_exp()

    # plot
    # plot("reader_q_size.csv", "reader_q_size")
    # plot("worker_q_size.csv", "worker_q_size")
    # plot("writer_q_size.csv", "writer_q_size")
    # plot("cc_lo_p.csv", "cc_lo_p")
    # plot("cc_hi_p.csv", "cc_hi_p")
    # plot("producer_num.csv", "producer_num")
    # plot("cc_period.csv", "cc_period (ms)", "log")
    # plot("qsize.csv", "reader, worker and writer queue size")
