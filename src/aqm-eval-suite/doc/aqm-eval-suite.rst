AQM Evaluation Suite
--------------------

.. include:: replace.txt
.. highlight:: cpp

.. heading hierarchy:
   ------------- Chapter
   ************* Section (#.#)
   ============= Subsection (#.#.#)
   ############# Paragraph (no number)


The AQM Evaluation Suite is an automated framework for comparing the performance of
ns-3 queue disciplines based on the scenarios mentioned in RFC 7928. It includes
simulation setup, topology creation, traffic generation, program execution, results
collection and their graphical representation.


Introducing the AQM Evaluation Suite
************************************

AQM Evaluation Suite automates the cycle of simulation setup to results collection
in ns-3. It closely follows the recommendations provided in RFC 7928 to setup the
simulation scenarios and collects results in the suggested formats. A user can
choose to either run an individual scenario from the set of scenarios provided in
the suite, or run all scenarios at once. Results for each scenario are systematically
stored in text and graphical formats.

The following provides more details about the architecture of the suite, users
interaction with the suite, its scope and limitations, and steps to extend the
suite for evaluating new AQM algorithms.

Architecture of AQM Evaluation Suite
************************************

The suite is implemented in ``src/aqm-eval-suite`` directory. 

Model
=====

``src/aqm-eval-suite/model`` contains an implementation of the following three
primary classes:

* class :cpp:class:`EvaluationTopology`: This class has three major functionalities:

  * Creating the topology: It sets up a point-to-point dumbbell topology by using
    :cpp:class:`PointToPointDumbbellHelper`: with required number of nodes, and
    configures the data rate and for all the links. It also installs the desired queue
    discipline on the router node.

  * Installing application on nodes: it provides an API for configuration of
    applications. This API takes the application parameters such as data rate,
    packet size, transport protocol, initial congestion window in case of TCP,
    maximum bandwidth and one way delay of the channels.

  * Getting the metrics of interest from the experiment: It uses the traces
    sources provided by different classes of ns-3 for the calculation of metrics.
    The metrics recommended in the RFC are queue-delay, goodput, throughput and
    number of drops.

* class :cpp:class:`EvalApp`: This class is based on :cpp:class:`OnOffApplication`:
  and is used for generating TCP and UDP traffic in the suite. The native class
  :cpp:class:`OnOffApplication`: in ns-3 creates sockets at the start time of an
  application. Thus, to configure different values for parameters like initial
  congestion window in TCP is non-trivial, since they cannot be configured before
  the socket is created and after the application starts. To overcome this,
  :cpp:class:`EvalApp`: is implemented on the same principles as that of the
  :cpp:class:`OnOffApplication`: in which a socket is created and the application
  is started only after its parameters are configured.

* class :cpp:class:`EvalTimestampTag`: This is a subclass of :cpp:class:`Tag`: and
  has been developed to fetch the queue delay information from :cpp:class:`QueueDisc`:.
  When a packet is enqueued by the QueueDisc, this tag is added with a timestamp
  (as the enqueue time) and when the packet is dequeued, the queue delay is computed
  as the difference between the dequeue time and the enqueue time.

Helper
======

``src/aqm-eval-suite/helper`` contains an implementation of the following class:

* class :cpp:class:`ScenarioImpl`: This class implements the following two methods:

  * ``ScenarioImpl::CreateScenario ()``: This is a virtual function implemented by
    each scenario according to the topology and traffic profiles mentioned in the RFC.

  * ``ScenarioImpl::RunSimulation ()``: This method takes the scenario created by
    each subclass and runs them with all the queue disciplines available in ns-3.

Utils
=====

``src/aqm-eval-suite/utils`` directory provides four Python scripts that take
performance metrics computed in the suite as input, and generate a graph with
Queuing Delay as the X-axis against Goodput as the Y-axis. The graph depicts an
ellipse which is plotted as per the guidelines mentioned in the RFC and [Remy].
The co-variance between the queuing delay and goodput is determined by the
orientation of the ellipse, and helps to analyze the effect of traffic load on
Goodput and Queuing Delay.

Examples
========

``src/aqm-eval-suite/examples`` directory provides a set of programs, each
corresponding to a specific scenario listed in RFC 7928. Each program can be
run individually. Alternatively, `aqm-eval-suite-runner.cc` allows the user
to run all scenarios at once.

User interaction with the suite
*******************************

Users can learn about the list of scenarios available in the suite from
``src/aqm-eval/examples`` directory. The programs can be run in a usual way.
For example, ``aggressive-transport-sender.cc`` is equivalent to the scenario
described in Section 5.2 of the RFC. Assuming examples have been enabled
during configure, the following commands would run ``aggressive-transport-sender.cc``

::

  $ ./waf --run "aqm-eval-suite-runner --number=5.2"

or

::

  $ ./waf --run "aqm-eval-suite-runner --name=AggressiveTransportSender"

To run all scenarios at once, the following command could be used:

::

  $ ./waf --run "aqm-eval-suite-runner --name=All"

Simulating additional AQM algorithms using this suite
*****************************************************

* By default, the suite evaluates AQM algorithms implemented in |ns3|. To
  simulate additional AQM algorithms, such as the ones designed by the user,
  the ``addAQM`` method of ``ScenarioImpl`` can be used in the scenarios
  available in ``src/aqm-eval-suite/examples``. For example, to add a new AQM
  of typeId ``ns3::ExampleQueueDisc`` in ``aggressive-transport-sender.cc``,
  ``CreateScenario`` method can be modified as shown in the code below:

.. code-block:: c++

  EvaluationTopology
  AggressiveTransportSender::CreateScenario (std::string aqm)
  {
    .
    .
    addAQM ("ns3::ExampleQueueDisc");
    EvaluationTopology et ("AggressiveTransportSender", nflow, pointToPoint, aqm, 1460);
    .
    .
  }

Scope and limitations of the suite
**********************************

* All scenarios described in Section 5, 6 and 8 of RFC 7928 are supported.

* Scenarios listed in Section 7 and 9 are not yet supported.

* Currently, the suite cannot be used to study the interaction of queue disciplines
  with Explicit Congestion Notification (ECN) and Scheduling Algorithms.

* Multi-AQM scenarios are not yet supported.

Packages Required for Processing Metrics and Graphing
*****************************************************

Following are the packages required for the suite and their installation instruction in Ubuntu

* python-pip: apt-get install python-pip

* python numpy: pip install numpy

* gnuplot: apt-get install gnuplot-qt

* imagemagick (optional package for ns-3): apt-get install imagemagick
