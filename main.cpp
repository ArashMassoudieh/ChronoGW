#include "Tracer.h"
#include "Well.h"
#include "GWA.h"
#include <iostream>
#include <fstream>
#include "GA.h"
#include "MCMC.h"
#include "levenbergmarquardt.h"

void example_tracer_output()
{
    std::cout << "=== Tracer Parameter Output Example ===\n" << std::endl;

    CTracer tracer("3H");
    tracer.setDecayRate(0.056261946);
    tracer.setRetardation(1.0);
    tracer.setOldWaterConcentration(5.0);
    tracer.setConstantInput(10.0);
    TimeSeries<double> atmospheric_record("../../Inputs/3H.txt");
    tracer.setInput(atmospheric_record);
    tracer.getInput().writefile("output/3H_record.csv");
    // Write to console
    std::cout << tracer.parametersToString() << std::endl;

    // Or write to file
    std::ofstream file("tracer_params.txt");
    tracer.writeInfo(file);
    file.close();

    std::cout << "Tracer parameters written to tracer_params.txt\n" << std::endl;
}

void example_well_output()
{
    std::cout << "=== Well Parameter Output Example ===\n" << std::endl;

    CWell well("Well-8");
    well.setDistributionType("exponential");
    well.setParameters({30.0});  // Mean age
    well.setFractionOld(0.3);
    well.setAgeOld(5000.0);
    well.setFractionModern(0.4);

    // Create distribution
    well.createDistribution(200.0, 100, 0.02);

    // Write to console
    std::cout << well.parametersToString() << std::endl;
    well.getYoungAgeDistribution().writefile("output/age_dist_exp.txt");
    // Or write to file
    std::ofstream file("well_params.txt");

    well.writeInfo(file);
    file.close();

    std::cout << "Well parameters written to well_params.txt\n" << std::endl;
}

void example_model_output()
{
    std::cout << "=== Full Model Output Example ===\n" << std::endl;

    // Load model from config file
    CGWA model("../../IG_nitrate_tracers.txt");

    // Write complete model summary
    std::cout << model.parametersToString() << std::endl;

    // Write to file
    std::ofstream file("model_summary.txt");
    model.writeModelSummary(file);
    file.close();

    // Write just parameter values (useful for optimization output)
    std::ofstream param_file("current_parameters.txt");
    model.writeParameterValues(param_file);
    param_file.close();

    std::cout << "Model summary written to model_summary.txt" << std::endl;
    std::cout << "Parameter values written to current_parameters.txt\n" << std::endl;
}

void example_during_optimization()
{
    std::cout << "=== Parameter Output During Optimization ===\n" << std::endl;

    CGWA model("../../IG_nitrate_tracers.txt");

    // Simulate optimization iteration
    std::vector<double> new_params = {45.0, 0.25, 6000.0};  // Example values
    model.setAllParameterValues(new_params);

    // After each iteration, log parameters
    std::ofstream log("optimization_log.txt", std::ios::app);
    log << "Iteration 1:\n";
    model.writeParameterValues(log);
    log << "Log-likelihood: " << model.calculateLogLikelihood() << "\n\n";
    log.close();

    std::cout << "Optimization progress logged to optimization_log.txt\n" << std::endl;
}

int main()
{
    try {
        CGWA system("../../Single_well.txt");
        system.exportToFile("output/Inputfile.txt");
        CGA<CGWA> ga(&system);
        CMCMC<CGWA> mcmc(&system);
        LevenbergMarquardt<CGWA> lm(&system);

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
