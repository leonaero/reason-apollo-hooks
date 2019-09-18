type queryError = {. "message": string};

type queryVariant('a) =
  | Data('a)
  | Error(queryError)
  | Loading
  | NoData;

/**
 *
 *  apollo-client/src/core/ObservableQuery.ts
 */
[@bs.deriving abstract]
type updateQueryOptions = {
  [@bs.optional]
  fetchMoreResult: Js.Json.t,
  [@bs.optional]
  variables: Js.Json.t,
};

type updateQueryT = (Js.Json.t, updateQueryOptions) => Js.Json.t;

type refetch('a) = (~variables: Js.Json.t=?, unit) => Js.Promise.t('a);
type queryResult('a) = {
  data: option('a),
  loading: bool,
  error: option(queryError),
  refetch: refetch('a),
  fetchMore:
    (~variables: Js.Json.t=?, ~updateQuery: updateQueryT, unit) =>
    Js.Promise.t(unit),
  networkStatus: Types.networkStatus,
};

/**
 * apollo-client/src/core/watchQueryOptions.ts
 */
[@bs.deriving abstract]
type fetchMoreOptions = {
  [@bs.optional]
  variables: Js.Json.t,
  updateQuery: updateQueryT,
};

[@bs.module "graphql-tag"] external gql: ReasonApolloTypes.gql = "default";

[@bs.deriving abstract]
type options = {
  [@bs.optional]
  variables: Js.Json.t,
  [@bs.optional]
  client: ApolloClient.generatedApolloClient,
  [@bs.optional]
  notifyOnNetworkStatusChange: bool,
  [@bs.optional]
  fetchPolicy: string,
  [@bs.optional]
  errorPolicy: string,
  [@bs.optional]
  pollInterval: int,
};

[@bs.module "@apollo/react-hooks"]
external useQueryJs:
  (ReasonApolloTypes.queryString, options) =>
  {
    .
    "data": Js.Nullable.t(Js.Json.t),
    "loading": bool,
    "error": Js.Nullable.t(queryError),
    [@bs.meth]
    "refetch": Js.Nullable.t(Js.Json.t) => Js.Promise.t(Js.Json.t),
    [@bs.meth] "fetchMore": fetchMoreOptions => Js.Promise.t(unit),
    "networkStatus": Js.Nullable.t(int),
  } =
  "useQuery";

let useQuery =
    (
      ~query,
      ~client=?,
      ~notifyOnNetworkStatusChange=?,
      ~fetchPolicy=?,
      ~errorPolicy=?,
      ~pollInterval=?,
      (),
    ) => {
  let jsResult =
    useQueryJs(
      gql(. query##query),
      options(
        ~variables=query##variables,
        ~client?,
        ~notifyOnNetworkStatusChange?,
        ~fetchPolicy=?fetchPolicy->Belt.Option.map(Types.fetchPolicyToJs),
        ~errorPolicy=?errorPolicy->Belt.Option.map(Types.errorPolicyToJs),
        ~pollInterval?,
        (),
      ),
    );

  let parse: Js.Json.t => 'response = query##parse;
  let getData = obj =>
    obj
    ->Js.Json.decodeObject
    ->Belt.Option.flatMap(x => Js.Dict.get(x, "data"))
    ->Belt.Option.getExn;

  let result =
    React.useMemo1(
      () =>
        {
          data:
            jsResult##data
            ->Js.Nullable.toOption
            ->Belt.Option.flatMap(data =>
                switch (parse(data)) {
                | parsedData => Some(parsedData)
                | exception _ => None
                }
              ),
          loading: jsResult##loading,
          error: jsResult##error->Js.Nullable.toOption,
          networkStatus: Types.toNetworkStatus(jsResult##networkStatus),
          refetch: (~variables=?, ()) =>
            jsResult##refetch(Js.Nullable.fromOption(variables))
            |> Js.Promise.then_(result =>
                 parse(result->getData) |> Js.Promise.resolve
               ),
          fetchMore: (~variables=?, ~updateQuery, ()) =>
            jsResult##fetchMore(
              fetchMoreOptions(~variables?, ~updateQuery, ()),
            ),
        },
      [|jsResult|],
    );

  (
    switch (result) {
    | {loading: true} => Loading
    | {error: Some(error)} => Error(error)
    | {data: Some(data)} => Data(data)
    | _ => NoData
    },
    result,
  );
};